#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "fisica/ModeloBiotDissipativo.hpp"
#include "fisica/ModeloBiotJKD.hpp"
#include "fisica/ModeloBiotNaoDissipativo.hpp"
#include "numerico/SolucionadorEquacoesPoroelasticas.hpp"
#include "simulacao/SimuladorOndasPoroelasticas.hpp"

namespace {

namespace cor {
constexpr const char* reset = "\033[0m";
constexpr const char* negritoCiano = "\033[1;36m";
constexpr const char* amarelo = "\033[1;33m";
constexpr const char* verde = "\033[1;32m";
constexpr const char* vermelho = "\033[1;31m";
} // namespace cor

void abrirPagina(int numero, const std::string& titulo) {
    std::cout << "\033[2J\033[H"
              << cor::negritoCiano
              << "============================================================\n"
              << "     SIMULADOR ACADÊMICO DE ONDAS TERMOPOROELÁSTICAS\n"
              << "  PÁGINA " << numero << " DE 4 — " << titulo << '\n'
              << "============================================================\n"
              << cor::reset;
}

std::string removerEspacos(const std::string& texto) {
    const std::size_t primeiro = texto.find_first_not_of(" \t\r\n");
    if (primeiro == std::string::npos) {
        return {};
    }
    const std::size_t ultimo = texto.find_last_not_of(" \t\r\n");
    return texto.substr(primeiro, ultimo - primeiro + 1U);
}

bool pertenceAoIntervalo(
    double valor,
    double minimo,
    bool minimoInclusivo,
    double maximo,
    bool maximoInclusivo) {
    const bool acimaMinimo = minimoInclusivo ? valor >= minimo : valor > minimo;
    const bool abaixoMaximo = maximoInclusivo ? valor <= maximo : valor < maximo;
    return std::isfinite(valor) && acimaMinimo && abaixoMaximo;
}

double solicitarValorReal(
    const std::string& rotulo,
    const std::string& unidade,
    double valorPadrao,
    double minimo,
    bool minimoInclusivo,
    double maximo = std::numeric_limits<double>::infinity(),
    bool maximoInclusivo = true) {
    while (true) {
        std::cout << cor::amarelo << "  " << rotulo;
        if (!unidade.empty()) {
            std::cout << " (" << unidade << ")";
        }
        std::cout << " [" << std::defaultfloat << std::setprecision(6)
                  << valorPadrao << "]: " << std::flush;

        std::string entrada;
        if (!std::getline(std::cin, entrada)) {
            std::cout << cor::reset;
            throw std::runtime_error("Não foi possível ler os dados da simulação.");
        }
        std::cout << cor::reset;
        entrada = removerEspacos(entrada);
        if (entrada.empty()) {
            return valorPadrao;
        }

        try {
            std::size_t caracteresLidos = 0U;
            const double valor = std::stod(entrada, &caracteresLidos);
            if (caracteresLidos == entrada.size() && pertenceAoIntervalo(
                    valor, minimo, minimoInclusivo, maximo, maximoInclusivo)) {
                return valor;
            }
        } catch (const std::exception&) {
            // A mensagem unica abaixo cobre formato invalido e estouro numerico.
        }
        std::cout << cor::vermelho
                  << "  Valor inválido ou fora do intervalo físico permitido.\n"
                  << cor::reset;
    }
}

int solicitarNumeroPontos() {
    constexpr int valorPadrao = 25;
    constexpr int maximoPontos = 100000;
    while (true) {
        std::cout << cor::amarelo
                  << "  Quantidade de pontos da malha [" << valorPadrao
                  << "]: " << std::flush;
        std::string entrada;
        if (!std::getline(std::cin, entrada)) {
            std::cout << cor::reset;
            throw std::runtime_error("Não foi possível ler os dados da simulação.");
        }
        std::cout << cor::reset;
        entrada = removerEspacos(entrada);
        if (entrada.empty()) {
            return valorPadrao;
        }

        try {
            std::size_t caracteresLidos = 0U;
            const long long valor = std::stoll(entrada, &caracteresLidos);
            if (caracteresLidos == entrada.size() && valor >= 2 &&
                valor <= maximoPontos) {
                return static_cast<int>(valor);
            }
        } catch (const std::exception&) {
            // A mensagem unica abaixo cobre formato invalido e estouro numerico.
        }
        std::cout << cor::vermelho
                  << "  Valor inválido. Informe um inteiro entre 2 e "
                  << maximoPontos << ".\n" << cor::reset;
    }
}

PropriedadesMeioPoroso solicitarPropriedades() {
    const PropriedadesMeioPoroso padrao =
        PropriedadesMeioPoroso::arenitoSaturadoComAgua();
    PropriedadesMeioPoroso meio = padrao;
    const double infinito = std::numeric_limits<double>::infinity();

    abrirPagina(1, "MATRIZ POROSA");
    std::cout <<
                 "  Referência: arenito homogêneo, isotrópico e saturado\n"
                 "  com água. Pressione ENTER para aceitar cada padrão.\n"
                 "  Para outra rocha, substitua os valores usando unidades SI.\n"
                 "  Use ponto como separador decimal.\n\n";

    meio.setDensidadeFaseSolida(solicitarValorReal(
        "Densidade da fase sólida", "kg/m³", padrao.getDensidadeFaseSolida(),
        0.0, false));
    meio.setPorosidade(solicitarValorReal(
        "Porosidade", "0 a 1", padrao.getPorosidade(),
        0.0, false, 1.0, false));
    meio.setTortuosidade(solicitarValorReal(
        "Tortuosidade", "adimensional", padrao.getTortuosidade(),
        1.0, true));
    meio.setPermeabilidade(solicitarValorReal(
        "Permeabilidade estática", "m²", padrao.getPermeabilidade(),
        0.0, false));
    meio.setModuloCisalhamento(solicitarValorReal(
        "Módulo de cisalhamento μ", "Pa", padrao.getModuloCisalhamento(),
        0.0, false));
    meio.setParametroLame(solicitarValorReal(
        "Parâmetro de Lamé λ", "Pa", padrao.getParametroLame(),
        0.0, false));
    meio.setModuloBiot(solicitarValorReal(
        "Módulo de Biot M", "Pa", padrao.getModuloBiot(),
        0.0, false));
    meio.setParametroBiotWillis(solicitarValorReal(
        "Parâmetro de Biot-Willis α", "0 a 1", padrao.getParametroBiotWillis(),
        0.0, false, 1.0, true));
    meio.setExpoenteCimentacao(solicitarValorReal(
        "Expoente de cimentação n", "adimensional",
        padrao.getExpoenteCimentacao(), 0.0, false));

    abrirPagina(2, "FLUIDO SATURANTE E GEOMETRIA VISCOSA");
    meio.setDensidadeFluido(solicitarValorReal(
        "Densidade do fluido", "kg/m³", padrao.getDensidadeFluido(),
        0.0, false));
    meio.setViscosidadeFluido(solicitarValorReal(
        "Viscosidade dinâmica do fluido", "Pa·s",
        padrao.getViscosidadeFluido(), 0.0, false));
    meio.setComprimentoCaracteristicoViscoso(solicitarValorReal(
        "Comprimento característico viscoso Λ", "m",
        padrao.getComprimentoCaracteristicoViscoso(), 0.0, false));

    abrirPagina(3, "PROPRIEDADES TÉRMICAS");
    meio.setCalorEspecifico(solicitarValorReal(
        "Calor específico do meio saturado", "J/(kg·K)",
        padrao.getCalorEspecifico(), 0.0, false));
    meio.setCondutividadeTermica(solicitarValorReal(
        "Condutividade térmica", "W/(m·K)",
        padrao.getCondutividadeTermica(), 0.0, false));
    meio.setTempoRelaxacaoTermica(solicitarValorReal(
        "Tempo de relaxação térmica τ₀", "s",
        padrao.getTempoRelaxacaoTermica(), 0.0, false));
    meio.setCoeficienteTermicoSolido(solicitarValorReal(
        "Coeficiente de tensão térmica do sólido βs", "Pa/K",
        padrao.getCoeficienteTermicoSolido(), 0.0, true, infinito, true));
    meio.setCoeficienteTermicoFluido(solicitarValorReal(
        "Coeficiente de tensão térmica do fluido βf", "Pa/K",
        padrao.getCoeficienteTermicoFluido(), 0.0, true, infinito, true));
    meio.setTemperaturaReferencia(solicitarValorReal(
        "Temperatura absoluta de referência Θ₀", "K",
        padrao.getTemperaturaReferencia(), 0.0, false));
    return meio;
}

ParametrosSimulacao solicitarParametrosSimulacao() {
    constexpr double frequenciaInicialPadrao = 1.0;
    constexpr double frequenciaFinalPadrao = 10000.0;

    ParametrosSimulacao parametros{};
    const PropriedadesMeioPoroso meio = solicitarPropriedades();

    abrirPagina(4, "SIMULAÇÃO");
    std::cout <<
                 "  A malha de frequências é logarítmica.\n"
                 "  Pressione ENTER para aceitar cada valor padrão.\n\n";

    const double frequenciaInicial = solicitarValorReal(
        "Frequência inicial", "Hz", frequenciaInicialPadrao, 0.0, false);
    const double frequenciaFinal = solicitarValorReal(
        "Frequência final", "Hz", frequenciaFinalPadrao,
        frequenciaInicial, false);
    const int numeroPontos = solicitarNumeroPontos();

    parametros.setFrequenciaInicial(frequenciaInicial);
    parametros.setFrequenciaFinal(frequenciaFinal);
    parametros.setNumeroPontos(numeroPontos);
    parametros.setPropriedadesMeioPoroso(meio);

    const double frequenciaCritica =
        SolucionadorEquacoesPoroelasticas::frequenciaCriticaHz(meio);
    std::cout << cor::verde
              << "\n------------------------------------------------------------\n"
                 "  CONFIGURAÇÃO DEFINIDA\n"
              << std::setprecision(12) << std::defaultfloat
              << "  Faixa: " << frequenciaInicial << " Hz a "
              << frequenciaFinal << " Hz\n"
              << "  Pontos: " << numeroPontos << " (malha logarítmica)\n"
              << "  Frequência crítica estimada: " << frequenciaCritica << " Hz\n"
              << cor::reset;
    if (frequenciaFinal < frequenciaCritica) {
        std::cout << cor::vermelho
                  << "  Aviso: a faixa termina antes da frequência crítica;\n"
                     "  a diferença entre Darcy e JKD poderá ser pequena.\n"
                  << cor::reset;
    }
    std::cout << cor::verde
              << "============================================================\n"
              << cor::reset;
    return parametros;
}

std::unique_ptr<ModeloPoroelastico> selecionarModelo(const std::string& nome) {
    if (nome == "non_dissipative") {
        return std::make_unique<ModeloBiotNaoDissipativo>();
    }
    if (nome == "dissipative") {
        return std::make_unique<ModeloBiotDissipativo>();
    }
    if (nome == "jkd") {
        return std::make_unique<ModeloBiotJKD>();
    }
    throw std::runtime_error("Modelo físico inválido.");
}

std::string solicitarModelo() {
    std::cout << cor::negritoCiano << "\n"
                 "============================================================\n"
                 "                 ESCOLHA DO MODELO FÍSICO\n"
                 "============================================================\n"
                 "\n"
                 "  [A]  TERMO-BIOT SEM DISSIPAÇÃO VISCOSA\n"
                 "       O acoplamento e a dissipação térmica permanecem ativos.\n"
                 "\n"
                 "  [B]  TERMO-BIOT-DARCY\n"
                 "       Arrasto viscoso constante no regime de baixa frequência.\n"
                 "\n"
                 "  [C]  TERMO-BIOT-JKD\n"
                 "       Permeabilidade dinâmica em uma ampla faixa de frequência.\n"
                 "\n"
                 "------------------------------------------------------------\n"
              << cor::reset;

    while (true) {
        std::cout << cor::amarelo
                  << "  Escolha uma opção [A/B/C] [padrão: C]: " << std::flush;
        std::string opcao;
        if (!std::getline(std::cin, opcao)) {
            std::cout << cor::reset;
            throw std::runtime_error("Não foi possível ler a opção do modelo.");
        }
        std::cout << cor::reset;
        opcao = removerEspacos(opcao);
        if (opcao.empty()) {
            opcao = "C";
        }
        if (opcao.size() == 1U) {
            opcao.front() = static_cast<char>(
                std::toupper(static_cast<unsigned char>(opcao.front())));
        }
        if (opcao == "A") {
            return "non_dissipative";
        }
        if (opcao == "B") {
            return "dissipative";
        }
        if (opcao == "C") {
            return "jkd";
        }
        std::cout << cor::vermelho
                  << "  Opção inválida. Digite apenas A, B ou C.\n"
                  << cor::reset;
    }
}

void mostrarResumo(const ResultadoSimulacao& resultado) {
    std::cout << cor::verde
              << "\nResumo da simulação (primeiros pontos)\n"
                 "T representa a onda térmica; VT é uma velocidade em m/s.\n\n";
    std::cout << std::setw(14) << "freq(Hz)"
              << std::setw(14) << "VP1(m/s)"
              << std::setw(14) << "VP2(m/s)"
              << std::setw(14) << "VT(m/s)"
              << std::setw(14) << "aP1(Np/m)"
              << std::setw(14) << "aP2(Np/m)"
              << std::setw(14) << "aT(Np/m)" << '\n'
              << cor::reset;
    const std::size_t limite =
        std::min<std::size_t>(resultado.getFrequencias().size(), 8U);
    for (std::size_t indice = 0U; indice < limite; ++indice) {
        std::cout << std::scientific << std::setprecision(6)
                  << std::setw(14) << resultado.getFrequencias()[indice]
                  << std::setw(14) << resultado.getVelocidadeFaseP1()[indice]
                  << std::setw(14) << resultado.getVelocidadeFaseP2()[indice]
                  << std::setw(14) << resultado.getVelocidadeFaseT()[indice]
                  << std::setw(14) << resultado.getAtenuacaoP1()[indice]
                  << std::setw(14) << resultado.getAtenuacaoP2()[indice]
                  << std::setw(14) << resultado.getAtenuacaoT()[indice] << '\n';
    }
    std::cout << std::defaultfloat;
}

void gerarGraficos(const ResultadoSimulacao& resultado) {
    constexpr std::size_t largura = 60U;
    constexpr std::size_t maximoPontosExibidos = 20U;
    const auto& frequencias = resultado.getFrequencias();

    auto desenharSerie = [&](const std::string& titulo, const std::string& unidade,
                             const std::vector<double>& valores) {
        const std::size_t total = std::min(frequencias.size(), valores.size());
        std::cout << "\n============================================================\n"
                  << titulo << '\n' << "Unidade: " << unidade << '\n';
        if (total == 0U) {
            std::cout << "Nenhum ponto disponível para esta série.\n";
            return;
        }

        const auto limites = std::minmax_element(
            valores.begin(), valores.begin() + static_cast<std::ptrdiff_t>(total));
        const double minimo = *limites.first;
        const double maximo = *limites.second;
        const double amplitude = maximo - minimo;
        const double magnitude = std::max(std::abs(minimo), std::abs(maximo));
        const double tolerancia = std::max(1.0, magnitude) * 1.0e-12;
        const bool constante =
            !std::isfinite(amplitude) || std::abs(amplitude) <= tolerancia;

        std::cout << std::scientific << std::setprecision(6)
                  << "Mínimo: " << minimo << " | Máximo: " << maximo << '\n';
        if (constante) {
            std::cout << "Série constante nesta faixa de frequências.\n";
        }

        const std::size_t exibidos = std::min(total, maximoPontosExibidos);
        for (std::size_t amostra = 0U; amostra < exibidos; ++amostra) {
            const std::size_t indice = exibidos == 1U ? 0U :
                static_cast<std::size_t>(std::round(
                    static_cast<double>(amostra) * static_cast<double>(total - 1U) /
                    static_cast<double>(exibidos - 1U)));
            const double valor = valores[indice];
            std::size_t posicao = largura / 2U;
            if (!constante && std::isfinite(valor)) {
                const double normalizado = (valor - minimo) / amplitude;
                posicao = static_cast<std::size_t>(std::round(
                    std::clamp(normalizado, 0.0, 1.0) *
                    static_cast<double>(largura - 1U)));
            }
            std::string linha(largura + 2U, ' ');
            linha.front() = '|';
            linha.back() = '|';
            linha[1U + posicao] = '*';
            std::cout << std::setw(13) << frequencias[indice] << " Hz "
                      << linha << " " << std::setw(13) << valor
                      << " " << unidade << '\n';
        }
        if (total > exibidos) {
            std::cout << "Exibidos " << exibidos << " de " << total
                      << " pontos distribuídos pela faixa.\n";
        }
        std::cout << std::defaultfloat;
    };

    std::cout << cor::verde << "\n\nGRÁFICOS ASCII DOS RESULTADOS\n"
              << cor::reset;
    desenharSerie("Velocidade de fase P1", "m/s", resultado.getVelocidadeFaseP1());
    desenharSerie("Velocidade de fase P2", "m/s", resultado.getVelocidadeFaseP2());
    desenharSerie("Velocidade da onda térmica T", "m/s", resultado.getVelocidadeFaseT());
    desenharSerie("Atenuação P1", "Np/m", resultado.getAtenuacaoP1());
    desenharSerie("Atenuação P2", "Np/m", resultado.getAtenuacaoP2());
    desenharSerie("Atenuação térmica T", "Np/m", resultado.getAtenuacaoT());
}

bool solicitarNovaSimulacao() {
    std::cout << cor::negritoCiano
              << "\n============================================================\n"
                 "  Deseja fazer uma nova simulação?\n"
                 "============================================================\n"
              << cor::reset;

    while (true) {
        std::cout << cor::amarelo
                  << "  Digite S para Sim ou N para Não [S/N]: " << std::flush;

        std::string resposta;
        if (!std::getline(std::cin, resposta)) {
            std::cout << cor::reset;
            throw std::runtime_error("Não foi possível ler a opção de continuidade.");
        }
        std::cout << cor::reset;
        resposta = removerEspacos(resposta);

        if (resposta.size() == 1U) {
            resposta.front() = static_cast<char>(
                std::toupper(static_cast<unsigned char>(resposta.front())));
        }
        if (resposta == "S") {
            return true;
        }
        if (resposta == "N") {
            return false;
        }

        std::cout << cor::vermelho
                  << "  Opção inválida. Digite apenas S ou N.\n"
                  << cor::reset;
    }
}

} // namespace

int main(int argc, char*[]) {
    try {
        if (argc != 1) {
            throw std::runtime_error(
                "Este programa não recebe argumentos. Use o menu interativo.");
        }

        bool executarNovamente = false;
        do {
            const ParametrosSimulacao parametros = solicitarParametrosSimulacao();
            const std::string nomeModelo = solicitarModelo();
            auto modelo = selecionarModelo(nomeModelo);

            std::cout << cor::verde << "Modelo selecionado: ";
            if (nomeModelo == "non_dissipative") {
                std::cout << "Termo-Biot sem dissipação viscosa\n";
            } else if (nomeModelo == "dissipative") {
                std::cout << "Termo-Biot-Darcy\n";
            } else {
                std::cout << "Termo-Biot-JKD\n";
            }
            std::cout << cor::reset;

            SimuladorOndasPoroelasticas simulador(*modelo);
            const ResultadoSimulacao resultado = simulador.executar(parametros);
            mostrarResumo(resultado);
            gerarGraficos(resultado);
            executarNovamente = solicitarNovaSimulacao();
        } while (executarNovamente);

        std::cout << "\033[2J\033[H" << std::flush;
        return 0;
    } catch (const std::exception& erro) {
        std::cerr << cor::vermelho << "Erro: " << erro.what()
                  << cor::reset << '\n';
        return 1;
    }
}
