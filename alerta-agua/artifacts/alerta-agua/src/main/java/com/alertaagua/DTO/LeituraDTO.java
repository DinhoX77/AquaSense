package com.alertaagua.DTO;

public record LeituraDTO(
        int    altura,
        String status,
        String classe,
        String descricao,
        String horario,
        String data,
        int    leituras
) {}
