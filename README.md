<div align="center">

# 🌊 HidroAlerta

### Sistema Inteligente de Monitoramento de Alagamento

<p>
  <strong>Protótipo IoT com ESP32, sensores e aplicação web em Java Spring Boot para monitorar o nível da água em tempo real.</strong>
</p>

<p>
  <img src="https://img.shields.io/badge/Java-19-red?style=for-the-badge&logo=openjdk" />
  <img src="https://img.shields.io/badge/Spring%20Boot-3.2.5-brightgreen?style=for-the-badge&logo=springboot" />
  <img src="https://img.shields.io/badge/ESP32-IoT-blue?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Status-Prot%C3%B3tipo-orange?style=for-the-badge" />
</p>

</div>

---

## 📌 Sobre o projeto

O **HidroAlerta** é um projeto de protótipo desenvolvido para monitorar o nível da água e identificar possíveis riscos de alagamento.

A proposta utiliza um **ESP32** conectado a sensores para medir a altura da água em pontos estratégicos, como bueiros e postes. Os dados coletados são enviados para uma aplicação web desenvolvida em **Java com Spring Boot**, onde o usuário consegue acompanhar a situação em tempo real por meio de um painel simples, responsivo e intuitivo.

O sistema classifica automaticamente a situação da água em quatro níveis:

| Status | Condição |
| :--- | :--- |
| 🟢 Normal | Nível seguro da água |
| 🟡 Atenção | Água em nível elevado |
| 🟠 Risco | Possível risco de alagamento |
| 🔴 Alagado | Situação crítica de alagamento |

---

## 🎯 Objetivo

O objetivo do projeto é criar uma solução acessível, educativa e funcional para auxiliar no monitoramento de áreas com risco de alagamento.

Além disso, o projeto busca demonstrar na prática a integração entre:

- Internet das Coisas;
- sensores físicos;
- microcontroladores;
- desenvolvimento web;
- backend em Java;
- comunicação via API REST.

---

## 🧠 Como funciona

O funcionamento básico do sistema acontece da seguinte forma:

```mermaid
flowchart LR
    A[Sensor de nível da água] --> B[ESP32]
    B --> C[API Java Spring Boot]
    C --> D[Painel Web]
    D --> E[Usuário acompanha o status]
