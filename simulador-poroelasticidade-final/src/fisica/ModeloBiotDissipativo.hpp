#pragma once

#include <complex>

#include "ModeloPoroelastico.hpp"
#include "../numerico/SolucionadorEquacoesPoroelasticas.hpp"

// Termo-Biot de baixa frequencia com arrasto viscoso de Darcy.
class ModeloBiotDissipativo final : public ModeloPoroelastico {
public:
    PropriedadesOnda calcular(
        double frequencia, const PropriedadesMeioPoroso& meio) override {
        const double arrasto =
            meio.getViscosidadeFluido() / meio.getPermeabilidade();
        return SolucionadorEquacoesPoroelasticas::resolver(
            frequencia, meio, std::complex<double>(arrasto, 0.0));
    }
};
