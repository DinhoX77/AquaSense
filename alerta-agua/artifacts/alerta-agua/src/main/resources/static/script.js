/* ============================================================
   ALERTA ÁGUA — script.js
   Busca dados do backend Java (Spring Boot) via API REST
   Endpoint: GET /api/leitura
   ============================================================ */

var MAX_HISTORICO = 20;
var INTERVALO_MS  = 5000;

var historicoLinhas = [];

/* ---------- cores por status ---------- */

function getBarraColor(classe) {
  var cores = {
    normal:  '#2e7d32',
    atencao: '#f57f17',
    risco:   '#e65100',
    alagado: '#b71c1c'
  };
  return cores[classe] || '#2e7d32';
}

/* ---------- atualização dos elementos ---------- */

function atualizarPainel(dados) {
  /* Altura e barra */
  var pct = Math.min((dados.altura / 150) * 100, 100);

  var alturaEl = document.getElementById('altura-valor');
  alturaEl.textContent  = dados.altura;
  alturaEl.style.color  = getBarraColor(dados.classe);

  var barra = document.getElementById('barra-nivel');
  barra.style.width      = pct + '%';
  barra.style.background = getBarraColor(dados.classe);

  /* Status badge */
  var badge = document.getElementById('status-badge');
  badge.textContent = dados.status;
  badge.className   = 'status-badge ' + dados.classe;

  document.getElementById('status-desc').textContent = dados.descricao;

  /* Hora */
  document.getElementById('hora-valor').textContent = dados.horario;
  document.getElementById('hora-data').textContent  = dados.data;

  /* Sensor conectado */
  var dot = document.getElementById('sensor-dot');
  var txt = document.getElementById('sensor-status-txt');

  if (dados.classe === 'alagado' || dados.classe === 'risco') {
    dot.className = 'sensor-dot alerta';
    txt.textContent = 'Sensor ativo — ALERTA!';
  } else {
    dot.className = 'sensor-dot conectado';
    txt.textContent = 'Sensor ativo e conectado';
  }

  /* Contagem de leituras */
  document.getElementById('contagem-leituras').textContent = '📋 Leituras: ' + dados.leituras;
}

/* ---------- alerta de risco/alagamento ---------- */

function gerenciarAlerta(dados) {
  var banner = document.getElementById('alerta-banner');
  var texto  = document.getElementById('alerta-texto');

  if (dados.classe === 'risco' || dados.classe === 'alagado') {
    if (dados.classe === 'alagado') {
      texto.textContent = '🚨 ALAGAMENTO! Procure um local seguro imediatamente.';
    } else {
      texto.textContent = '⚠️ Atenção! Risco de alagamento detectado na região.';
    }
    banner.classList.remove('hidden');
  } else {
    banner.classList.add('hidden');
  }
}

/* ---------- histórico ---------- */

function adicionarHistorico(dados) {
  historicoLinhas.unshift(dados);

  if (historicoLinhas.length > MAX_HISTORICO) {
    historicoLinhas.pop();
  }

  var tbody = document.getElementById('historico-body');
  tbody.innerHTML = '';

  historicoLinhas.forEach(function(item) {
    var tr    = document.createElement('tr');
    var tHora = document.createElement('td');
    var tAlt  = document.createElement('td');
    var tSt   = document.createElement('td');

    tHora.textContent = item.horario;
    tAlt.textContent  = item.altura + ' cm';

    var tag = document.createElement('span');
    tag.className   = 'tag-status tag-' + item.classe;
    tag.textContent = item.status;
    tSt.appendChild(tag);

    tr.appendChild(tHora);
    tr.appendChild(tAlt);
    tr.appendChild(tSt);
    tbody.appendChild(tr);
  });
}

/* ---------- chamada ao backend Java ---------- */

function buscarLeitura() {
  fetch('/sensor/leitura')
    .then(function(response) {
      if (!response.ok) {
        throw new Error('Erro na resposta do servidor: ' + response.status);
      }
      return response.json();
    })
    .then(function(dados) {
      atualizarPainel(dados);
      gerenciarAlerta(dados);
      adicionarHistorico(dados);
    })
    .catch(function(erro) {
      console.error('Erro ao buscar leitura do backend Java:', erro);
      var dot = document.getElementById('sensor-dot');
      var txt = document.getElementById('sensor-status-txt');
      dot.className   = 'sensor-dot';
      txt.textContent = 'Erro de conexão com o servidor';
    });
}

/* ---------- fechar alerta manualmente ---------- */

document.getElementById('fechar-alerta').addEventListener('click', function() {
  document.getElementById('alerta-banner').classList.add('hidden');
});

/* ---------- inicialização ---------- */

buscarLeitura();
setInterval(buscarLeitura, INTERVALO_MS);
