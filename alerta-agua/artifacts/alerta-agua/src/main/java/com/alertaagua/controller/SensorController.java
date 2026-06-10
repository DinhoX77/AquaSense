package com.alertaagua.controller;

import com.alertaagua.DTO.LeituraDTO;
import com.alertaagua.DTO.LeituraRequestDTO;
import com.alertaagua.service.SensorService;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/sensor")
public class SensorController {

    private final SensorService sensorService;

    public SensorController(SensorService sensorService) {
        this.sensorService = sensorService;
    }

    @GetMapping("/leitura")
    public LeituraDTO getLeitura() {
        return sensorService.buscarUltimaLeitura();
    }

    @PostMapping("/leitura")
    public LeituraDTO receberLeitura(@RequestBody LeituraRequestDTO dados) {
        return sensorService.salvarLeituraRecebida(
                dados.altura(),
                dados.alagadoRua()
        );
    }

    @GetMapping("/historico")
    public List<LeituraDTO> getHistorico() {
        return sensorService.listarHistorico();
    }
}