#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <limits>
#include <stdexcept>
#include <string>

#include "../modelo/PropriedadesMeioPoroso.hpp"
#include "../modelo/PropriedadesOnda.hpp"

// Soluciona a relacao de dispersao termoporoelastica longitudinal 1D.
// A convencao harmonica adotada e exp(i * (k*x - omega*t)).
class SolucionadorEquacoesPoroelasticas {
private:
    using Complexo = std::complex<double>;
    using Polinomio = std::array<Complexo, 4U>; // coeficientes em ordem crescente

    struct Coeficientes {
        double H{};
        double C{};
        double M{};
        double densidadeTotal{};
        double densidadeFluido{};
        double beta{};
        double betaFluido{};
        double capacidadeTermicaVolumetrica{};
        double condutividadeTermica{};
        double temperaturaReferencia{};
        Complexo densidadeInercial{};
        Complexo relaxacaoTermica{};
    };

    struct ModoOnda {
        double velocidade{};
        double atenuacao{};
    };

    static constexpr double pi = 3.141592653589793238462643383279502884;

    static bool finito(const Complexo& valor) {
        return std::isfinite(valor.real()) && std::isfinite(valor.imag());
    }

    static void exigirPositivo(double valor, const std::string& nome) {
        if (!std::isfinite(valor) || valor <= 0.0) {
            throw std::runtime_error("Propriedade fisica invalida: " + nome + ".");
        }
    }

    static void exigirNaoNegativo(double valor, const std::string& nome) {
        if (!std::isfinite(valor) || valor < 0.0) {
            throw std::runtime_error("Propriedade fisica invalida: " + nome + ".");
        }
    }

    static void validar(double frequencia, const PropriedadesMeioPoroso& meio) {
        exigirPositivo(frequencia, "frequencia");
        exigirPositivo(meio.getDensidadeFaseSolida(), "densidade da fase solida");
        exigirPositivo(meio.getPermeabilidade(), "permeabilidade");
        exigirPositivo(meio.getTortuosidade(), "tortuosidade");
        exigirPositivo(meio.getModuloCisalhamento(), "modulo de cisalhamento");
        exigirPositivo(meio.getParametroLame(), "parametro de Lame");
        exigirPositivo(meio.getModuloBiot(), "modulo de Biot");
        exigirPositivo(meio.getDensidadeFluido(), "densidade do fluido");
        exigirPositivo(meio.getViscosidadeFluido(), "viscosidade do fluido");
        exigirPositivo(
            meio.getComprimentoCaracteristicoViscoso(), "comprimento viscoso");
        exigirPositivo(meio.getExpoenteCimentacao(), "expoente de cimentacao");
        exigirPositivo(meio.getCalorEspecifico(), "calor especifico");
        exigirPositivo(meio.getCondutividadeTermica(), "condutividade termica");
        exigirPositivo(
            meio.getTempoRelaxacaoTermica(), "tempo de relaxacao termica");
        exigirPositivo(meio.getTemperaturaReferencia(), "temperatura de referencia");
        exigirNaoNegativo(
            meio.getCoeficienteTermicoSolido(), "coeficiente termico do solido");
        exigirNaoNegativo(
            meio.getCoeficienteTermicoFluido(), "coeficiente termico do fluido");

        if (!std::isfinite(meio.getPorosidade()) || meio.getPorosidade() <= 0.0 ||
            meio.getPorosidade() >= 1.0) {
            throw std::runtime_error("A porosidade deve estar entre 0 e 1.");
        }
        if (meio.getTortuosidade() < 1.0) {
            throw std::runtime_error("A tortuosidade deve ser maior ou igual a 1.");
        }
        if (!std::isfinite(meio.getParametroBiotWillis()) ||
            meio.getParametroBiotWillis() <= 0.0 ||
            meio.getParametroBiotWillis() > 1.0) {
            throw std::runtime_error(
                "O parametro de Biot-Willis deve estar entre 0 (exclusivo) e 1.");
        }
    }

    static double fatorFormacao(const PropriedadesMeioPoroso& meio) {
        const double fator = meio.getTortuosidade() *
            std::pow(meio.getPorosidade(), -meio.getExpoenteCimentacao());
        exigirPositivo(fator, "fator de formacao");
        return fator;
    }

    static Coeficientes calcularCoeficientes(
        double omega,
        const PropriedadesMeioPoroso& meio,
        const Complexo& arrastoViscoso) {
        const double phi = meio.getPorosidade();
        const double rhoF = meio.getDensidadeFluido();
        const double M = meio.getModuloBiot();
        const double alfa = meio.getParametroBiotWillis();
        const double lambda = meio.getParametroLame();
        const double mu = meio.getModuloCisalhamento();
        const double betaF = meio.getCoeficienteTermicoFluido();
        const double beta = meio.getCoeficienteTermicoSolido() + alfa * betaF;

        Coeficientes c{};
        c.H = lambda + 2.0 * mu + alfa * alfa * M;
        c.C = alfa * M;
        c.M = M;
        c.densidadeTotal =
            (1.0 - phi) * meio.getDensidadeFaseSolida() + phi * rhoF;
        c.densidadeFluido = rhoF;
        c.beta = beta;
        c.betaFluido = betaF;
        c.capacidadeTermicaVolumetrica =
            c.densidadeTotal * meio.getCalorEspecifico();
        c.condutividadeTermica = meio.getCondutividadeTermica();
        c.temperaturaReferencia = meio.getTemperaturaReferencia();
        c.densidadeInercial = rhoF * fatorFormacao(meio) +
            Complexo(0.0, 1.0) * arrastoViscoso / omega;
        c.relaxacaoTermica = Complexo(
            meio.getTempoRelaxacaoTermica(), 1.0 / omega);

        const double determinanteElastico = c.H * c.M - c.C * c.C;
        exigirPositivo(determinanteElastico, "determinante elastico");
        const double determinanteInercial =
            c.densidadeTotal * (rhoF * fatorFormacao(meio)) - rhoF * rhoF;
        exigirPositivo(determinanteInercial, "determinante inercial");
        if (!finito(c.densidadeInercial) || c.densidadeInercial.real() <= 0.0) {
            throw std::runtime_error("Densidade inercial complexa invalida.");
        }
        return c;
    }

    static Polinomio somar(const Polinomio& a, const Polinomio& b) {
        Polinomio resultado{};
        for (std::size_t i = 0U; i < resultado.size(); ++i) {
            resultado[i] = a[i] + b[i];
        }
        return resultado;
    }

    static Polinomio subtrair(const Polinomio& a, const Polinomio& b) {
        Polinomio resultado{};
        for (std::size_t i = 0U; i < resultado.size(); ++i) {
            resultado[i] = a[i] - b[i];
        }
        return resultado;
    }

    static Polinomio multiplicar(const Polinomio& a, const Polinomio& b) {
        Polinomio resultado{};
        for (std::size_t i = 0U; i < a.size(); ++i) {
            for (std::size_t j = 0U; j + i < resultado.size(); ++j) {
                resultado[i + j] += a[i] * b[j];
            }
        }
        return resultado;
    }

    static Polinomio linear(const Complexo& constante, const Complexo& coeficiente) {
        return Polinomio{constante, coeficiente, Complexo{}, Complexo{}};
    }

    static Polinomio montarPolinomio(const Coeficientes& c) {
        // A variavel do polinomio e q = p^2 = (k/omega)^2.
        // A terceira amplitude foi escalada para eliminar sqrt(q) da matriz.
        const Polinomio a11 = linear(-c.densidadeTotal, c.H);
        const Polinomio a12 = linear(-c.densidadeFluido, c.C);
        const Polinomio a13 = linear(0.0, c.beta);
        const Polinomio a21 = a12;
        const Polinomio a22 = linear(-c.densidadeInercial, c.M);
        const Polinomio a23 = linear(0.0, c.betaFluido);
        const Polinomio a31 = linear(
            c.beta * c.temperaturaReferencia * c.relaxacaoTermica, 0.0);
        const Polinomio a32 = a31;
        const Polinomio a33 = linear(
            -c.capacidadeTermicaVolumetrica * c.relaxacaoTermica,
            c.condutividadeTermica);

        const Polinomio termo1 = multiplicar(
            a11, subtrair(multiplicar(a22, a33), multiplicar(a23, a32)));
        const Polinomio termo2 = multiplicar(
            a12, subtrair(multiplicar(a21, a33), multiplicar(a23, a31)));
        const Polinomio termo3 = multiplicar(
            a13, subtrair(multiplicar(a21, a32), multiplicar(a22, a31)));
        Polinomio resultado = somar(subtrair(termo1, termo2), termo3);

        double escala = 0.0;
        for (const Complexo& coeficiente : resultado) {
            if (!finito(coeficiente)) {
                throw std::runtime_error("Coeficiente nao finito na equacao cubica.");
            }
            escala = std::max(escala, std::abs(coeficiente));
        }
        exigirPositivo(escala, "escala da equacao cubica");
        std::transform(
            resultado.begin(), resultado.end(), resultado.begin(),
            [escala](const Complexo& coeficiente) {
                return coeficiente / escala;
            });
        if (std::abs(resultado[3]) <= std::numeric_limits<double>::min()) {
            throw std::runtime_error("Equacao termoporoelastica degenerada.");
        }
        return resultado;
    }

    static Complexo avaliar(const Polinomio& p, const Complexo& x) {
        return ((p[3] * x + p[2]) * x + p[1]) * x + p[0];
    }

    static Complexo avaliarDerivada(const Polinomio& p, const Complexo& x) {
        return (3.0 * p[3] * x + 2.0 * p[2]) * x + p[1];
    }

    static std::array<Complexo, 2U> resolverQuadratica(
        const Complexo& a, const Complexo& b, const Complexo& c) {
        const Complexo discriminante = std::sqrt(b * b - 4.0 * a * c);
        const Complexo nMais = -b + discriminante;
        const Complexo nMenos = -b - discriminante;
        const Complexo numerador =
            std::abs(nMais) >= std::abs(nMenos) ? nMais : nMenos;
        if (std::abs(a) == 0.0 || std::abs(numerador) == 0.0) {
            throw std::runtime_error("Equacao quadratica auxiliar degenerada.");
        }
        const Complexo raiz1 = numerador / (2.0 * a);
        const Complexo raiz2 = c / (a * raiz1);
        return {raiz1, raiz2};
    }

    static std::array<Complexo, 3U> estimativasIniciais(
        const Coeficientes& c) {
        const Complexo a = c.H * c.M - c.C * c.C;
        const Complexo b = -(
            c.H * c.densidadeInercial + c.M * c.densidadeTotal -
            2.0 * c.C * c.densidadeFluido);
        const Complexo d =
            c.densidadeTotal * c.densidadeInercial -
            c.densidadeFluido * c.densidadeFluido;
        const auto raizesMecanicas = resolverQuadratica(a, b, d);
        const Complexo raizTermica =
            c.capacidadeTermicaVolumetrica * c.relaxacaoTermica /
            c.condutividadeTermica;
        return {raizesMecanicas[0], raizesMecanicas[1], raizTermica};
    }

    static double residuoRelativo(
        const Polinomio& p, const Complexo& raiz) {
        const double modulo = std::abs(raiz);
        double escala = 0.0;
        double potencia = 1.0;
        for (const Complexo& coeficiente : p) {
            escala += std::abs(coeficiente) * potencia;
            potencia *= modulo;
        }
        return std::abs(avaliar(p, raiz)) /
            std::max(escala, std::numeric_limits<double>::min());
    }

    static std::array<Complexo, 3U> resolverCubica(
        const Polinomio& p, std::array<Complexo, 3U> raizes) {
        constexpr int maximoIteracoes = 100;
        constexpr double tolerancia = 1.0e-13;
        bool convergiu = false;

        for (int iteracao = 0; iteracao < maximoIteracoes; ++iteracao) {
            const auto anteriores = raizes;
            double maiorCorrecao = 0.0;
            bool denominadorValido = true;
            for (std::size_t i = 0U; i < raizes.size(); ++i) {
                Complexo denominador = p[3];
                for (std::size_t j = 0U; j < raizes.size(); ++j) {
                    if (i != j) {
                        denominador *= anteriores[i] - anteriores[j];
                    }
                }
                if (!finito(denominador) ||
                    std::abs(denominador) <= std::numeric_limits<double>::min()) {
                    denominadorValido = false;
                    break;
                }
                const Complexo correcao = avaliar(p, anteriores[i]) / denominador;
                raizes[i] = anteriores[i] - correcao;
                maiorCorrecao = std::max(
                    maiorCorrecao,
                    std::abs(correcao) / (1.0 + std::abs(raizes[i])));
            }
            if (!denominadorValido) {
                break;
            }
            if (maiorCorrecao <= tolerancia) {
                convergiu = true;
                break;
            }
        }

        if (!convergiu) {
            throw std::runtime_error(
                "As raizes termoporoelasticas nao convergiram.");
        }

        // Polimento independente por Newton reduz o residuo final.
        for (Complexo& raiz : raizes) {
            for (int iteracao = 0; iteracao < 8; ++iteracao) {
                const Complexo derivada = avaliarDerivada(p, raiz);
                if (!finito(derivada) ||
                    std::abs(derivada) <= std::numeric_limits<double>::min()) {
                    break;
                }
                const Complexo correcao = avaliar(p, raiz) / derivada;
                raiz -= correcao;
                if (std::abs(correcao) <=
                    tolerancia * (1.0 + std::abs(raiz))) {
                    break;
                }
            }
            if (!finito(raiz) || residuoRelativo(p, raiz) > 1.0e-8) {
                throw std::runtime_error("Raiz termoporoelastica numericamente invalida.");
            }
        }
        return raizes;
    }

    static ModoOnda calcularModo(const Complexo& lentidaoQuadratica, double omega) {
        Complexo lentidao = std::sqrt(lentidaoQuadratica);
        if (lentidao.real() < 0.0) {
            lentidao = -lentidao;
        }
        const double escala = std::max(1.0, std::abs(lentidao));
        if (lentidao.imag() < 0.0 && lentidao.imag() >= -1.0e-12 * escala) {
            lentidao = Complexo(lentidao.real(), 0.0);
        }
        if (!finito(lentidao) || lentidao.real() <= 0.0 || lentidao.imag() < 0.0) {
            throw std::runtime_error("Nao foi possivel selecionar uma onda passiva.");
        }

        ModoOnda modo{};
        modo.velocidade = 1.0 / lentidao.real();
        modo.atenuacao = omega * lentidao.imag();
        if (!std::isfinite(modo.velocidade) || modo.velocidade <= 0.0 ||
            !std::isfinite(modo.atenuacao) || modo.atenuacao < 0.0) {
            throw std::runtime_error("Resultado termoporoelastico nao finito.");
        }
        return modo;
    }

public:
    static double frequenciaAngular(double frequencia) {
        return 2.0 * pi * frequencia;
    }

    static double frequenciaCriticaHz(const PropriedadesMeioPoroso& meio) {
        const double omegaCritico = meio.getViscosidadeFluido() /
            (meio.getPermeabilidade() * fatorFormacao(meio) *
             meio.getDensidadeFluido());
        return omegaCritico / (2.0 * pi);
    }

    static double numeroPride(const PropriedadesMeioPoroso& meio) {
        const double comprimento = meio.getComprimentoCaracteristicoViscoso();
        return 4.0 * fatorFormacao(meio) * meio.getPermeabilidade() /
            (comprimento * comprimento);
    }

    static double frequenciaAngularRelaxacaoJKD(
        const PropriedadesMeioPoroso& meio) {
        const double pride = numeroPride(meio);
        exigirPositivo(pride, "numero de Pride");
        return 2.0 * pi * frequenciaCriticaHz(meio) / pride;
    }

    static PropriedadesOnda resolver(
        double frequencia,
        const PropriedadesMeioPoroso& meio,
        const Complexo& arrastoViscoso) {
        validar(frequencia, meio);
        if (!finito(arrastoViscoso) || arrastoViscoso.real() < 0.0) {
            throw std::runtime_error("Arrasto viscoso complexo invalido.");
        }

        const double omega = frequenciaAngular(frequencia);
        const Coeficientes coeficientes =
            calcularCoeficientes(omega, meio, arrastoViscoso);
        const Polinomio polinomio = montarPolinomio(coeficientes);
        const auto estimativas = estimativasIniciais(coeficientes);
        const auto raizes = resolverCubica(polinomio, estimativas);

        // O modo termico e associado por continuidade a raiz termica do
        // sistema desacoplado. Ordenar os tres modos apenas pela velocidade
        // troca incorretamente P2 e T em meios de permeabilidade muito baixa.
        std::size_t indiceTermico = 0U;
        double menorDistancia = std::numeric_limits<double>::infinity();
        for (std::size_t i = 0U; i < raizes.size(); ++i) {
            const double escala = std::max({
                std::abs(raizes[i]), std::abs(estimativas[2]),
                std::numeric_limits<double>::min()});
            const double distancia =
                std::abs(raizes[i] - estimativas[2]) / escala;
            if (distancia < menorDistancia) {
                menorDistancia = distancia;
                indiceTermico = i;
            }
        }

        const ModoOnda modoTermico = calcularModo(raizes[indiceTermico], omega);
        std::array<ModoOnda, 2U> modosMecanicos{};
        std::size_t indiceMecanico = 0U;
        for (std::size_t i = 0U; i < raizes.size(); ++i) {
            if (i != indiceTermico) {
                modosMecanicos[indiceMecanico] = calcularModo(raizes[i], omega);
                ++indiceMecanico;
            }
        }
        std::sort(
            modosMecanicos.begin(), modosMecanicos.end(),
            [](const ModoOnda& a, const ModoOnda& b) {
            return a.velocidade > b.velocidade;
        });

        PropriedadesOnda onda{};
        onda.setVelocidadeFaseP1(modosMecanicos[0].velocidade);
        onda.setVelocidadeFaseP2(modosMecanicos[1].velocidade);
        onda.setVelocidadeFaseT(modoTermico.velocidade);
        onda.setAtenuacaoP1(modosMecanicos[0].atenuacao);
        onda.setAtenuacaoP2(modosMecanicos[1].atenuacao);
        onda.setAtenuacaoT(modoTermico.atenuacao);
        return onda;
    }
};
