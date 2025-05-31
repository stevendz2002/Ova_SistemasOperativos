package libRoundRobin;

/**
 *  Clase que representa las funciones de la biblioteca dinamica JNI
 */
public class RoundRobin {
    public native String procesoRoundRobin(String json);

    public RoundRobin() {
        try {
            System.loadLibrary("RoundRobin");
            System.out.println("Biblioteca cargada exitosamente!");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Carga de la biblioteca fallida: " + e.getMessage());
        }
    }
}