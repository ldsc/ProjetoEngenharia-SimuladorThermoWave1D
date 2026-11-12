#pragma once

// Transporta velocidades e atenuacoes das ondas longitudinais P1, P2 e
// termica T para uma unica frequencia. T nao representa uma onda cisalhante.
class PropriedadesOnda {
private:
    double velocidadeFaseP1{};
    double velocidadeFaseP2{};
    double velocidadeFaseT{};
    double atenuacaoP1{};
    double atenuacaoP2{};
    double atenuacaoT{};

public:
    double getVelocidadeFaseP1() const {
        return velocidadeFaseP1;
    }

    void setVelocidadeFaseP1(double valor) {
        velocidadeFaseP1 = valor;
    }

    double getVelocidadeFaseP2() const {
        return velocidadeFaseP2;
    }

    void setVelocidadeFaseP2(double valor) {
        velocidadeFaseP2 = valor;
    }

    double getVelocidadeFaseT() const {
        return velocidadeFaseT;
    }

    void setVelocidadeFaseT(double valor) {
        velocidadeFaseT = valor;
    }

    double getAtenuacaoP1() const {
        return atenuacaoP1;
    }

    void setAtenuacaoP1(double valor) {
        atenuacaoP1 = valor;
    }

    double getAtenuacaoP2() const {
        return atenuacaoP2;
    }

    void setAtenuacaoP2(double valor) {
        atenuacaoP2 = valor;
    }

    double getAtenuacaoT() const {
        return atenuacaoT;
    }

    void setAtenuacaoT(double valor) {
        atenuacaoT = valor;
    }
};
