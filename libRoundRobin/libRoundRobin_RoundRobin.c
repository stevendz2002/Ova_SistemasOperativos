#include <stdio.h>
#include <jni.h>
#include <cjson/cJSON.h>
#include "libRoundRobin_RoundRobin.h"
#include <stdlib.h>

typedef struct {
    int id_proceso;
    int tiempo_rafaga;
    int prioridad;
    int tiempo_llegada;
    int tiempo_restante;
    int finish;
    int tiempo_inicio;
    int tiempo_fin;
    int tiempo_espera;
    int tiempo_sistema;
} Proceso;

static int ejecutar_round_robin(Proceso *procesos, int cantidad, int quantum, cJSON *array_diagrama) {
    int reloj = 0;
    int terminados = 0;
    int *visitado = calloc(cantidad, sizeof(int));

    while (terminados < cantidad) {
        int hubo_ejecucion = 0;
        for (int i = 0; i < cantidad; i++) {
            if (procesos[i].tiempo_llegada <= reloj && procesos[i].tiempo_restante > 0) {
                hubo_ejecucion = 1;
                if (!visitado[i]) {
                    procesos[i].tiempo_inicio = reloj;
                    visitado[i] = 1;
                }

                int ejec = (procesos[i].tiempo_restante >= quantum) ? quantum : procesos[i].tiempo_restante;

                for (int j = 0; j < ejec; j++) {
                    for (int k = 0; k < cantidad; k++) {
                        cJSON *fila = cJSON_GetArrayItem(array_diagrama, k);
                        int estado = 0;
                        if (k == i) estado = 2;
                        else if (procesos[k].tiempo_llegada <= reloj && procesos[k].tiempo_restante > 0) estado = 1;
                        cJSON_AddItemToArray(fila, cJSON_CreateNumber(estado));
                    }
                    reloj++;
                }

                procesos[i].tiempo_restante -= ejec;
                if (procesos[i].tiempo_restante == 0) {
                    procesos[i].finish = 1;
                    procesos[i].tiempo_fin = reloj;
                    procesos[i].tiempo_sistema = procesos[i].tiempo_fin - procesos[i].tiempo_llegada;
                    procesos[i].tiempo_espera = procesos[i].tiempo_sistema - procesos[i].tiempo_rafaga;
                    terminados++;
                }
            }
        }

        if (!hubo_ejecucion) {
            for (int k = 0; k < cantidad; k++) {
                cJSON *fila = cJSON_GetArrayItem(array_diagrama, k);
                cJSON_AddItemToArray(fila, cJSON_CreateNumber(0));
            }
            reloj++;
        }
    }

    free(visitado);
    return reloj;
}

JNIEXPORT jstring JNICALL Java_libRoundRobin_RoundRobin_procesoRoundRobin(JNIEnv *env, jobject obj, jstring json) {
    const char *json_input = (*env)->GetStringUTFChars(env, json, 0);
    cJSON *entrada = cJSON_Parse(json_input);
    cJSON *objeto_final = cJSON_CreateObject();
    cJSON *sistema_espera = cJSON_CreateObject();

    cJSON *array_procesos = cJSON_GetObjectItem(entrada, "procesos");
    cJSON *quantum_obj = cJSON_GetObjectItem(entrada, "quantum");

    if (!array_procesos || !quantum_obj) {
        cJSON_AddStringToObject(sistema_espera, "error", "Faltan procesos o quantum");
        cJSON_AddItemToObject(objeto_final, "Error", sistema_espera);
        char *resultado = cJSON_Print(objeto_final);
        jstring salida = (*env)->NewStringUTF(env, resultado);
        free(resultado);
        cJSON_Delete(entrada);
        cJSON_Delete(objeto_final);
        (*env)->ReleaseStringUTFChars(env, json, json_input);
        return salida;
    }

    int size = cJSON_GetArraySize(array_procesos);
    int quantum = quantum_obj->valueint;
    Proceso *procesos = (Proceso *)calloc(size, sizeof(Proceso));

    for (int i = 0; i < size; i++) {
        cJSON *item = cJSON_GetArrayItem(array_procesos, i);
        procesos[i].id_proceso = cJSON_GetObjectItem(item, "id")->valueint;
        procesos[i].tiempo_llegada = cJSON_GetObjectItem(item, "llegada")->valueint;
        procesos[i].tiempo_rafaga = cJSON_GetObjectItem(item, "rafaga")->valueint;
        procesos[i].prioridad = cJSON_GetObjectItem(item, "prioridad")->valueint;
        procesos[i].tiempo_restante = procesos[i].tiempo_rafaga;
    }

    cJSON *array_diagrama = cJSON_CreateArray();
    for (int i = 0; i < size; i++) {
        cJSON *fila = cJSON_CreateArray();
        cJSON_AddItemToArray(array_diagrama, fila);
    }

    int tiempo_total = ejecutar_round_robin(procesos, size, quantum, array_diagrama);

    for (int i = 0; i < size; i++) {
        cJSON *proceso_n = cJSON_CreateObject();
        cJSON_AddNumberToObject(proceso_n, "tiempo_sistema", procesos[i].tiempo_sistema);
        cJSON_AddNumberToObject(proceso_n, "tiempo_espera", procesos[i].tiempo_espera);
        char nombre_proceso[15];
        sprintf(nombre_proceso, "%d", procesos[i].id_proceso);
        cJSON_AddItemToObject(sistema_espera, nombre_proceso, proceso_n);
    }

    cJSON_AddItemToObject(objeto_final, "Procesos", sistema_espera);
    cJSON_AddItemToObject(objeto_final, "Diagrama", array_diagrama);

    char *resultado = cJSON_Print(objeto_final);
    jstring salida = (*env)->NewStringUTF(env, resultado);

    cJSON_Delete(entrada);
    cJSON_Delete(objeto_final);
    free(procesos);
    free(resultado);
    (*env)->ReleaseStringUTFChars(env, json, json_input);
    return salida;
}
