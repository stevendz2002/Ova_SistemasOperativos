package com.denkitronik.springbootjni;

import com.fasterxml.jackson.databind.ObjectMapper;
import libRoundRobin.ProcesosDTO;
import libRoundRobin.RoundRobin;
import org.springframework.web.bind.annotation.*;
@CrossOrigin
@RestController
@RequestMapping({"/api/sistemas-operativos"})
public class JNIRestController {
    private RoundRobin procesoRoundRobin = new RoundRobin();

    public JNIRestController() {
    }

    @PostMapping({"/roundrobin/{quantum}"})
    public String roundRobin(@PathVariable int quantum, @RequestBody ProcesosDTO[] jsonDTO) {
        StringBuilder jsonBuilder = new StringBuilder();
        jsonBuilder.append("{");
        jsonBuilder.append("\"procesos\":[");

        for (int i = 0; i < jsonDTO.length; ++i) {
            jsonBuilder.append("{");
            jsonBuilder.append("\"id\":").append(jsonDTO[i].id()).append(",");
            jsonBuilder.append("\"llegada\":").append(jsonDTO[i].llegada()).append(",");
            jsonBuilder.append("\"rafaga\":").append(jsonDTO[i].rafaga()).append(",");
            jsonBuilder.append("\"prioridad\":").append(jsonDTO[i].prioridad());
            jsonBuilder.append("}");
            if (i < jsonDTO.length - 1) {
                jsonBuilder.append(",");
            }
        }

        jsonBuilder.append("],");
        jsonBuilder.append("\"quantum\":").append(quantum);
        jsonBuilder.append("}");
        String json = jsonBuilder.toString();
        return this.procesoRoundRobin.procesoRoundRobin(json);
    }
}
