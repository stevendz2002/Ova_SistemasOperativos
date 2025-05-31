package aplicacion;

import libRoundRobin.RoundRobin;
import java.util.Scanner;

public class Aplicacion {

    // Declaramos el objeto que representa la biblioteca JNI
    RoundRobin procesoRR = null;

    // Cargamos la biblioteca nativa
    static {
        System.loadLibrary("RoundRobin");
    }

    public Aplicacion() {
        this.procesoRR = new RoundRobin();

        Scanner scanner = new Scanner(System.in);

        System.out.print("Ingrese la cantidad de procesos: ");
        int cantidadProcesos = scanner.nextInt();

        // Construimos el JSON manualmente como una cadena
        StringBuilder jsonBuilder = new StringBuilder();
        jsonBuilder.append("{");
        jsonBuilder.append("\"procesos\":[");

        for (int i = 0; i < cantidadProcesos; i++) {
            System.out.println("Ingrese los datos del proceso " + (i + 1) + ":");
            System.out.print("ID: ");
            int id = scanner.nextInt();
            System.out.print("Tiempo de llegada: ");
            int llegada = scanner.nextInt();
            System.out.print("Tiempo de ráfaga: ");
            int rafaga = scanner.nextInt();
            System.out.print("Prioridad: ");
            int prioridad = scanner.nextInt();

            // Agregamos el proceso al JSON
            jsonBuilder.append("{");
            jsonBuilder.append("\"id\":").append(id).append(",");
            jsonBuilder.append("\"llegada\":").append(llegada).append(",");
            jsonBuilder.append("\"rafaga\":").append(rafaga).append(",");
            jsonBuilder.append("\"prioridad\":").append(prioridad);
            jsonBuilder.append("}");

            if (i < cantidadProcesos - 1) {
                jsonBuilder.append(",");
            }
        }

        jsonBuilder.append("],");

        System.out.print("Ingrese el quantum: ");
        int quantum = scanner.nextInt();
        jsonBuilder.append("\"quantum\":").append(quantum);
        jsonBuilder.append("}");

        // Convertimos el StringBuilder a una cadena JSON
        String json = jsonBuilder.toString();
	System.out.println(json);
        // Llamamos al método nativo
        String resultado = procesoRR.procesoRoundRobin(json);
        System.out.println("Resultado Round Robin:\n" + resultado);

        scanner.close();
    }

    public static void main(String[] args) {
        new Aplicacion();
    }
}
