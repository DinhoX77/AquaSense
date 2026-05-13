package com.alertaagua.repository;

import com.alertaagua.model.Leitura;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.Optional;

public interface LeituraRepository extends JpaRepository<Leitura, Long> {

    List<Leitura> findTop20ByOrderByDataHoraDesc();

    Optional<Leitura> findTopByOrderByDataHoraDesc();
}