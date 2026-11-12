#pragma once

#include <complex>

#include "ModeloPoroelastico.hpp"
#include "../numerico/SolucionadorEquacoesPoroelasticas.hpp"

// Termo-Biot-JKD com permeabilidade dinamica de Johnson-Koplik-Dashen.
class ModeloBiotJKD final : public ModeloPoroelastico {
public:
    PropriedadesOnda calcular(
        double frequencia, const PropriedadesMeioPoroso& meio) override {
        const double omega =
            SolucionadorEquacoesPoroelasticas::frequenciaAngular(frequencia);
        const double omegaRelaxacao =
            SolucionadorEquacoesPoroelasticas::frequenciaAngularRelaxacaoJKD(meio);
        const std::complex<double> unidadeImaginaria(0.0, 1.0);
        const std::complex<double> correcaoJKD =
            std::sqrt(1.0 - unidadeImaginaria * omega / omegaRelaxacao);
        const double arrastoDarcy =
            meio.getViscosidadeFluido() / meio.getPermeabilidade();
        const std::complex<double> arrastoDinamico =
            arrastoDarcy * correcaoJKD;
        return SolucionadorEquacoesPoroelasticas::resolver(
            frequencia, meio, arrastoDinamico);
    }
};
