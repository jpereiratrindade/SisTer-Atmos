# SisTer Atmos

**SisTer Atmos — inteligência climática e territorial para pesquisa.**

`system_id`: `sister_atmos`  
Linguagem: C++23  
Papel: subsistema independente do ecossistema SisTer.

## Estado atual

**A0 — Constituição.**

A0 registra identidade, fronteiras, linhagem, invariantes científicos,
contratos iniciais e um build C++23 mínimo. Nenhuma regra climática produtiva
é portada nesta fase.

## Princípio de migração

Sister-Clima 2.5 permanece como `legacy_reference` e oráculo temporário de
regressão. O objetivo da migração é equivalência comportamental quando a regra
legada estiver correta, ou diferença explicitamente justificada quando não
estiver.

## Praxis

`.hoa/`, `domain/`, `specs/`, `harness/` e `docs/experiments/` formam a memória
governada de engenharia. O runtime do Atmos não depende do Praxis.

## Verificação A0

```bash
./scripts/verify_a0.sh
```
