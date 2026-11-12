#pragma once

#include <complex>

#include "ModeloPoroelastico.hpp"
#include "../numerico/SolucionadorEquacoesPoroelasticas.hpp"

// Termo-Biot sem dissipacao viscosa. O acoplamento termico permanece ativo.
class ModeloBiotNaoDissipativo final : public ModeloPoroelastico {
public:
    PropriedadesOnda calcular(
        double frequencia, const PropriedadesMeioPoroso& meio) override {
        return SolucionadorEquacoesPoroelasticas::resolver(
            frequencia, meio, std::complex<double>(0.0, 0.0));
    }
};
