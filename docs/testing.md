# Testes

Validacoes automatizadas disponiveis:

- `tests/validate_chain.py`: build, layout da imagem e smoke test de boot.
- `tests/validate_ls.py`: prompt, execucao de `ls` e retorno ao shell.
- `tests/validate_cat.py`: leitura de arquivo por `cat`.
- `tests/validate_utils.py`: fluxo de utilitarios e escrita minima.

Comandos uteis:

```sh
make all
make test
python3 tests/validate_utils.py
```

Se um teste interativo falhar, consulte os logs em `build/*-serial.log`.
