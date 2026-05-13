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

    private static final DateTimeFormatter HORA_FMT = DateTimeFormatter.ofPattern("HH:mm:ss");
    private static final DateTimeFormatter DATA_FMT = DateTimeFormatter.ofPattern("dd/MM/yyyy");

    private final LeituraRepository leituraRepository;

    public SensorService(LeituraRepository leituraRepository) {
        this.leituraRepository = leituraRepository;
    }

    public synchronized LeituraDTO salvarLeituraRecebida(int altura) {
        StatusInfo status = calcularStatus(altura);

        Leitura leitura = new Leitura(
                altura,
                status.label(),
                status.classe(),
                status.descricao(),
                LocalDateTime.now()
        );

        leituraRepository.save(leitura);

        return converterParaDTO(leitura);
    }

    public LeituraDTO buscarUltimaLeitura() {
        return leituraRepository.findTopByOrderByDataHoraDesc()
                .map(this::converterParaDTO)
                .orElseGet(() -> criarLeituraInicial());
    }

    public List<LeituraDTO> listarHistorico() {
        return leituraRepository.findTop20ByOrderByDataHoraDesc()
                .stream()
                .map(this::converterParaDTO)
                .toList();
    }

    private LeituraDTO criarLeituraInicial() {
        StatusInfo status = calcularStatus(0);

        return new LeituraDTO(
                0,
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
                leitura.getStatus(),
                leitura.getClasse(),
                leitura.getDescricao(),
                leitura.getDataHora().format(HORA_FMT),
                leitura.getDataHora().format(DATA_FMT),
                (int) totalLeituras
        );
    }

    private StatusInfo calcularStatus(int altura) {
        if (altura < 30) {
            return new StatusInfo(
                    "Normal",
                    "normal",
                    "Nível da água dentro do esperado. Sem risco de alagamento."
            );
        } else if (altura < 60) {
            return new StatusInfo(
                    "Atenção",
                    "atencao",
                    "Nível elevado. Fique atento e acompanhe as atualizações."
            );
        } else if (altura < 90) {
            return new StatusInfo(
                    "Risco",
                    "risco",
                    "Nível crítico! Prepare-se para possível evacuação da área."
            );
        } else {
            return new StatusInfo(
                    "Alagado",
                    "alagado",
                    "ALAGAMENTO! Procure abrigo em local seguro imediatamente."
            );
        }
    }

    private record StatusInfo(String label, String classe, String descricao) {}
}