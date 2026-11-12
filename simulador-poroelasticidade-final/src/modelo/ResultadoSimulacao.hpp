#pragma once

#include <utility>
#include <vector>

// Cada indice associa uma frequencia aos resultados P1, P2 e termico T.
class ResultadoSimulacao {
private:
    std::vector<double> frequencias{};
    std::vector<double> velocidadeFaseP1{};
    std::vector<double> velocidadeFaseP2{};
    std::vector<double> velocidadeFaseT{};
    std::vector<double> atenuacaoP1{};
    std::vector<double> atenuacaoP2{};
    std::vector<double> atenuacaoT{};

public:
    const std::vector<double>& getFrequencias() const {
        return frequencias;
    }

    void setFrequencias(std::vector<double> valores) {
        frequencias = std::move(valores);
    }

    const std::vector<double>& getVelocidadeFaseP1() const {
        return velocidadeFaseP1;
    }

    void setVelocidadeFaseP1(std::vector<double> valores) {
        velocidadeFaseP1 = std::move(valores);
    }

    const std::vector<double>& getVelocidadeFaseP2() const {
        return velocidadeFaseP2;
    }

    void setVelocidadeFaseP2(std::vector<double> valores) {
        velocidadeFaseP2 = std::move(valores);
    }

    const std::vector<double>& getVelocidadeFaseT() const {
        return velocidadeFaseT;
    }

    void setVelocidadeFaseT(std::vector<double> valores) {
        velocidadeFaseT = std::move(valores);
    }

    const std::vector<double>& getAtenuacaoP1() const {
        return atenuacaoP1;
    }

    void setAtenuacaoP1(std::vector<double> valores) {
        atenuacaoP1 = std::move(valores);
    }

    const std::vector<double>& getAtenuacaoP2() const {
        return atenuacaoP2;
    }

    void setAtenuacaoP2(std::vector<double> valores) {
        atenuacaoP2 = std::move(valores);
    }

    const std::vector<double>& getAtenuacaoT() const {
        return atenuacaoT;
    }

    void setAtenuacaoT(std::vector<double> valores) {
        atenuacaoT = std::move(valores);
    }
};
