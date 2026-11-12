# Simulador Acadêmico de Ondas Termoporoelásticas

Simulador C++17, no domínio da frequência, para propagação longitudinal 1D em
um meio poroso homogêneo, isotrópico e totalmente saturado. Para cada
frequência, o programa calcula as velocidades de fase e as atenuações dos três
modos longitudinais acoplados:

- `P1`: onda compressiva rápida;
- `P2`: onda compressiva lenta;
- `T`: modo térmico.

`T` não é uma onda de cisalhamento e não representa a temperatura em um ponto.
É a raiz térmica da relação de dispersão termoporoelástica. O módulo de
cisalhamento `μ` continua necessário porque participa da rigidez longitudinal
`λ + 2μ`, embora o simulador não calcule uma onda transversal `S`.

Todas as propriedades são solicitadas pelo menu. Os valores da dissertação de
referência aparecem entre colchetes e podem ser aceitos com `ENTER`. O programa
não recebe argumentos, não lê `example.cfg` e não cria arquivo de resultados.

## Compilar e executar

Na raiz deste projeto:

```bash
mkdir -p build
g++ -std=c++17 -Wall -Wextra -Wpedantic src/main.cpp -o build/simulador
./build/simulador
```

No Windows com o `g++` do MSYS2 UCRT64:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++17 -Wall -Wextra -Wpedantic src/main.cpp -o build/simulador.exe
.\build\simulador.exe
```
