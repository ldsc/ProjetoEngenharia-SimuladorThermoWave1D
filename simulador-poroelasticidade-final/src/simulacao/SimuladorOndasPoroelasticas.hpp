#pragma once

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../fisica/ModeloPoroelastico.hpp"
#include "../modelo/ParametrosSimulacao.hpp"
#include "../modelo/ResultadoSimulacao.hpp"

// Recebe o modelo por referencia; sua vida util deve abranger a simulacao.
class SimuladorOndasPoroelasticas {
private:
    ModeloPoroelastico& modelo;

    static void validar(const ParametrosSimulacao& parametros) {
        if (parametros.getNumeroPontos() < 2 ||
            parametros.getNumeroPontos() > 100000) {
            throw std::runtime_error("A quantidade de pontos deve estar entre 2 e 100000.");
        }
        if (!std::isfinite(parametros.getFrequenciaInicial()) ||
            !std::isfinite(parametros.getFrequenciaFinal()) ||
            parametros.getFrequenciaInicial() <= 0.0 ||
            parametros.getFrequenciaFinal() <= parametros.getFrequenciaInicial()) {
            throw std::runtime_error("Faixa de frequencia invalida.");
        }
    }

public:
    explicit SimuladorOndasPoroelasticas(ModeloPoroelastico& modeloSelecionado)
        : modelo(modeloSelecionado) {}

    ResultadoSimulacao executar(const ParametrosSimulacao& parametros) {
        auto frequencias = gerarFrequencias(parametros);
        const std::size_t tamanho = frequencias.size();

        std::vector<double> velocidadeP1, velocidadeP2, velocidadeT;
        std::vector<double> atenuacaoP1, atenuacaoP2, atenuacaoT;
        velocidadeP1.reserve(tamanho);
        velocidadeP2.reserve(tamanho);
        velocidadeT.reserve(tamanho);
        atenuacaoP1.reserve(tamanho);
        atenuacaoP2.reserve(tamanho);
        atenuacaoT.reserve(tamanho);

        const auto& meio = parametros.getPropriedadesMeioPoroso();
        for (double frequencia : frequencias) {
            const PropriedadesOnda onda = modelo.calcular(frequencia, meio);
            velocidadeP1.push_back(onda.getVelocidadeFaseP1());
            velocidadeP2.push_back(onda.getVelocidadeFaseP2());
            velocidadeT.push_back(onda.getVelocidadeFaseT());
            atenuacaoP1.push_back(onda.getAtenuacaoP1());
            atenuacaoP2.push_back(onda.getAtenuacaoP2());
            atenuacaoT.push_back(onda.getAtenuacaoT());
        }

        ResultadoSimulacao resultado{};
        resultado.setFrequencias(std::move(frequencias));
        resultado.setVelocidadeFaseP1(std::move(velocidadeP1));
        resultado.setVelocidadeFaseP2(std::move(velocidadeP2));
        resultado.setVelocidadeFaseT(std::move(velocidadeT));
        resultado.setAtenuacaoP1(std::move(atenuacaoP1));
        resultado.setAtenuacaoP2(std::move(atenuacaoP2));
        resultado.setAtenuacaoT(std::move(atenuacaoT));
        return resultado;
    }

    static std::vector<double> gerarFrequencias(
        const ParametrosSimulacao& parametros) {
        validar(parametros);
        const double logInicial = std::log(parametros.getFrequenciaInicial());
        const double logFinal = std::log(parametros.getFrequenciaFinal());
        const double passoLogaritmico =
            (logFinal - logInicial) /
            static_cast<double>(parametros.getNumeroPontos() - 1);
        std::vector<double> frequencias;
        frequencias.reserve(static_cast<std::size_t>(parametros.getNumeroPontos()));
        for (int indice = 0; indice < parametros.getNumeroPontos(); ++indice) {
            const double expoente =
                logInicial + passoLogaritmico * static_cast<double>(indice);
            frequencias.push_back(std::exp(expoente));
        }
        frequencias.front() = parametros.getFrequenciaInicial();
        frequencias.back() = parametros.getFrequenciaFinal();
        return frequencias;
    }
};
