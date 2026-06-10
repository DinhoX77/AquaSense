package com.alertaagua.model;

import jakarta.persistence.*;
import java.time.LocalDateTime;

@Entity
public class Leitura {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    private int altura;

    private boolean alagadoRua;

    private String status;

    private String classe;

    private String descricao;

    private LocalDateTime dataHora;

    public Leitura() {}

    public Leitura(
            int altura,
            boolean alagadoRua,
            String status,
            String classe,
            String descricao,
            LocalDateTime dataHora
    ) {
        this.altura = altura;
        this.alagadoRua = alagadoRua;
        this.status = status;
        this.classe = classe;
        this.descricao = descricao;
        this.dataHora = dataHora;
    }

    public Long getId() {
        return id;
    }

    public int getAltura() {
        return altura;
    }

    public boolean isAlagadoRua() {
        return alagadoRua;
    }

    public String getStatus() {
        return status;
    }

    public String getClasse() {
        return classe;
    }

    public String getDescricao() {
        return descricao;
    }

    public LocalDateTime getDataHora() {
        return dataHora;
    }
}