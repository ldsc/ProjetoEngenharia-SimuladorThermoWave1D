#pragma once

#include "PropriedadesMeioPoroso.hpp"

// Define a malha logaritmica de frequencias e as propriedades do meio.
class ParametrosSimulacao {
private:
    double frequenciaInicial{};
    double frequenciaFinal{};
    int numeroPontos{};
    PropriedadesMeioPoroso propriedadesMeioPoroso{};

public:
    double getFrequenciaInicial() const {
        return frequenciaInicial;
    }

    void setFrequenciaInicial(double valor) {
        frequenciaInicial = valor;
    }

    double getFrequenciaFinal() const {
        return frequenciaFinal;
    }

    void setFrequenciaFinal(double valor) {
        frequenciaFinal = valor;
    }

    int getNumeroPontos() const {
        return numeroPontos;
    }

    void setNumeroPontos(int valor) {
        numeroPontos = valor;
    }

    const PropriedadesMeioPoroso& getPropriedadesMeioPoroso() const {
        return propriedadesMeioPoroso;
    }

    void setPropriedadesMeioPoroso(const PropriedadesMeioPoroso& valor) {
        propriedadesMeioPoroso = valor;
    }
};
