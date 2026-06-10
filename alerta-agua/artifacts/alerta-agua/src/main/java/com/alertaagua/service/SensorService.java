package com.alertaagua.service;

import com.alertaagua.DTO.LeituraDTO;
import com.alertaagua.model.Leitura;
import com.alertaagua.repository.LeituraRepository;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.List;

@Service
public class SensorService {

    private static final DateTimeFormatter HORA_FMT =
            DateTimeFormatter.ofPattern("HH:mm:ss");

    private static final DateTimeFormatter DATA_FMT =
            DateTimeFormatter.ofPattern("dd/MM/yyyy");

    // Altura interna total do bueiro da maquete
    private static final int ALTURA_MAXIMA_BUEIRO = 28;

    private final LeituraRepository leituraRepository;

    public SensorService(LeituraRepository leituraRepository) {
        this.leituraRepository = leituraRepository;
    }

    public synchronized LeituraDTO salvarLeituraRecebida(
            int altura,
            boolean alagadoRua
    ) {
        int alturaTratada = normalizarAltura(altura);

        StatusInfo status = calcularStatus(
                alturaTratada,
                alagadoRua
        );

        Leitura leitura = new Leitura(
                alturaTratada,
                alagadoRua,
                status.label(),
                status.classe(),
                status.descricao(),
                LocalDateTime.now()
        );

        leituraRepository.save(leitura);

        return converterParaDTO(leitura);
    }

    public LeituraDTO buscarUltimaLeitura() {
        return leituraRepository
                .findTopByOrderByDataHoraDesc()
                .map(this::converterParaDTO)
                .orElseGet(this::criarLeituraInicial);
    }

    public List<LeituraDTO> listarHistorico() {
        return leituraRepository
                .findTop20ByOrderByDataHoraDesc()
                .stream()
                .map(this::converterParaDTO)
                .toList();
    }

    private int normalizarAltura(int altura) {
        if (altura < 0) {
            return 0;
        }

        if (altura > ALTURA_MAXIMA_BUEIRO) {
            return ALTURA_MAXIMA_BUEIRO;
        }

        return altura;
    }

    private LeituraDTO criarLeituraInicial() {
        StatusInfo status = calcularStatus(0, false);

        return new LeituraDTO(
                0,
                false,
                status.label(),
                status.classe(),
                status.descricao(),
                "--:--:--",
                "--/--/----",
                0
        );
    }

    private LeituraDTO converterParaDTO(Leitura leitura) {
        long totalLeituras = leituraRepository.count();

        return new LeituraDTO(
                leitura.getAltura(),
                leitura.isAlagadoRua(),
                leitura.getStatus(),
                leitura.getClasse(),
                leitura.getDescricao(),
                leitura.getDataHora().format(HORA_FMT),
                leitura.getDataHora().format(DATA_FMT),
                (int) totalLeituras
        );
    }

    private StatusInfo calcularStatus(
            int altura,
            boolean alagadoRua
    ) {
        // O sensor da rua possui prioridade máxima.
        // Se ele detectou água, o bueiro já transbordou.
        if (alagadoRua) {
            return new StatusInfo(
                    "Alagado",
                    "alagado",
                    "ALAGAMENTO! Água detectada na rua. Procure um local seguro."
            );
        }

        if (altura < 8) {
            return new StatusInfo(
                    "Normal",
                    "normal",
                    "Nível da água dentro do esperado. Sem risco de alagamento."
            );
        }

        if (altura < 16) {
            return new StatusInfo(
                    "Atenção",
                    "atencao",
                    "Nível da água subindo. Acompanhe as atualizações."
            );
        }

        return new StatusInfo(
                "Risco",
                "risco",
                "Nível crítico no bueiro! Existe risco de transbordamento."
        );
    }

    private record StatusInfo(
            String label,
            String classe,
            String descricao
    ) {}
}