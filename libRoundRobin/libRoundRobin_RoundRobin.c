#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include <cjson/cJSON.h>
#include "libRoundRobin_RoundRobin.h"

typedef struct {
    int id_proceso;
    int tiempo_llegada;
    int tiempo_rafaga;
    int prioridad;
    int tiempo_restante;
    int tiempo_fin;
    int tiempo_sistema;
    int tiempo_espera;
} Proceso;

int comparar_llegada(const void *a, const void *b) {
    Proceso *p1 = (Proceso *)a;
    Proceso *p2 = (Proceso *)b;

    if (p1->tiempo_llegada != p2->tiempo_llegada)
        return p1->tiempo_llegada - p2->tiempo_llegada;
    return p1->id_proceso - p2->id_proceso;
}

static int ejecutar_round_robin(Proceso *procesos, int cantidad, int quantum, cJSON *array_diagrama) {
    int reloj = 0;
    int terminados = 0;
    int *cola = malloc(sizeof(int) * cantidad * 10); // espacio amplio
    int frente = 0, final = 0;
    int *en_cola = calloc(cantidad, sizeof(int));

    while (terminados < cantidad) {
        // Agregar a la cola procesos nuevos que hayan llegado
        for (int i = 0; i < cantidad; i++) {
            if (!en_cola[i] && procesos[i].tiempo_llegada <= reloj && procesos[i].tiempo_restante > 0) {
                cola[final++] = i;
                en_cola[i] = 1;
            }
        }

        if (frente == final) {
            // Tiempo muerto (ningún proceso disponible)
            for (int k = 0; k < cantidad; k++) {
                cJSON *fila = cJSON_GetArrayItem(array_diagrama, k);
                cJSON_AddItemToArray(fila, cJSON_CreateNumber(0));
            }
            reloj++;
            continue;
        }

        int idx = cola[frente++];
        int ejec = (procesos[idx].tiempo_restante >= quantum) ? quantum : procesos[idx].tiempo_restante;

        for (int t = 0; t < ejec; t++) {
            for (int k = 0; k < cantidad; k++) {
                cJSON *fila = cJSON_GetArrayItem(array_diagrama, k);
                int estado = 0;
                if (k == idx)
                    estado = 2;
                else if (procesos[k].tiempo_llegada <= reloj && procesos[k].tiempo_restante > 0)
                    estado = 1;
                cJSON_AddItemToArray(fila, cJSON_CreateNumber(estado));
            }
            reloj++;

            // Agregar procesos que llegan mientras se ejecuta
            for (int i = 0; i < cantidad; i++) {
                if (!en_cola[i] && procesos[i].tiempo_llegada <= reloj && procesos[i].tiempo_restante > 0) {
                    cola[final++] = i;
                    en_cola[i] = 1;
                }
            }
        }

        procesos[idx].tiempo_restante -= ejec;

        if (procesos[idx].tiempo_restante == 0) {
            procesos[idx].tiempo_fin = reloj;
            procesos[idx].tiempo_sistema = procesos[idx].tiempo_fin - procesos[idx].tiempo_llegada;
            procesos[idx].tiempo_espera = procesos[idx].tiempo_sistema - procesos[idx].tiempo_rafaga;
            terminados++;
        } else {
            cola[final++] = idx; // vuelve a la cola
        }
    }

    free(cola);
    free(en_cola);
    return reloj;
}

JNIEXPORT jstring JNICALL Java_libRoundRobin_RoundRobin_procesoRoundRobin(JNIEnv *env, jobject obj, jstring json) {
    const char *json_input = (*env)->GetStringUTFChars(env, json, 0);
    cJSON *entrada = cJSON_Parse(json_input);
    cJSON *respuesta = cJSON_CreateObject();
    cJSON *obj_procesos = cJSON_CreateObject();

    cJSON *array_procesos = cJSON_GetObjectItem(entrada, "procesos");
    cJSON *quantum_obj = cJSON_GetObjectItem(entrada, "quantum");

    if (!array_procesos || !quantum_obj) {
        cJSON_AddStringToObject(obj_procesos, "error", "Faltan procesos o quantum");
        cJSON_AddItemToObject(respuesta, "Error", obj_procesos);
        char *resultado = cJSON_Print(respuesta);
        jstring salida = (*env)->NewStringUTF(env, resultado);
        free(resultado);
        cJSON_Delete(entrada);
        cJSON_Delete(respuesta);
        (*env)->ReleaseStringUTFChars(env, json, json_input);
        return salida;
    }

    int cantidad = cJSON_GetArraySize(array_procesos);
    int quantum = quantum_obj->valueint;
    Proceso *procesos = calloc(cantidad, sizeof(Proceso));

    for (int i = 0; i < cantidad; i++) {
        cJSON *item = cJSON_GetArrayItem(array_procesos, i);
        procesos[i].id_proceso = cJSON_GetObjectItem(item, "id")->valueint;
        procesos[i].tiempo_llegada = cJSON_GetObjectItem(item, "llegada")->valueint;
        procesos[i].tiempo_rafaga = cJSON_GetObjectItem(item, "rafaga")->valueint;
        procesos[i].prioridad = cJSON_GetObjectItem(item, "prioridad")->valueint;
        procesos[i].tiempo_restante = procesos[i].tiempo_rafaga;
    }

    qsort(procesos, cantidad, sizeof(Proceso), comparar_llegada);

    cJSON *array_diagrama = cJSON_CreateArray();
    for (int i = 0; i < cantidad; i++)
        cJSON_AddItemToArray(array_diagrama, cJSON_CreateArray());

    ejecutar_round_robin(procesos, cantidad, quantum, array_diagrama);

    for (int i = 0; i < cantidad; i++) {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "tiempo_sistema", procesos[i].tiempo_sistema);
        cJSON_AddNumberToObject(obj, "tiempo_espera", procesos[i].tiempo_espera);

        char id_str[12];
        sprintf(id_str, "%d", procesos[i].id_proceso);
        cJSON_AddItemToObject(obj_procesos, id_str, obj);
    }

    cJSON_AddItemToObject(respuesta, "Procesos", obj_procesos);
    cJSON_AddItemToObject(respuesta, "Diagrama", array_diagrama);

    char *salida_final = cJSON_Print(respuesta);
    jstring resultado_final = (*env)->NewStringUTF(env, salida_final);

    free(salida_final);
    free(procesos);
    cJSON_Delete(entrada);
    cJSON_Delete(respuesta);
    (*env)->ReleaseStringUTFChars(env, json, json_input);
    return resultado_final;
}
