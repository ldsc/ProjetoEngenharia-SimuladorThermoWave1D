#pragma once

#include "../modelo/PropriedadesMeioPoroso.hpp"
#include "../modelo/PropriedadesOnda.hpp"

// Contrato comum aos tres modelos termoporoelasticos longitudinais 1D.
class ModeloPoroelastico {
public:
    virtual ~ModeloPoroelastico() = default;
    virtual PropriedadesOnda calcular(
        double frequencia, const PropriedadesMeioPoroso& meio) = 0;
};
