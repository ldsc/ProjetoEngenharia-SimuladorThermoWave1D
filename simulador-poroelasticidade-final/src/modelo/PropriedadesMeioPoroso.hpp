#pragma once

// Propriedades do meio termoporoelastico 1D em unidades do SI.
// Os valores-padrao representam o arenito saturado com agua usado na
// dissertacao de referencia. Todos podem ser substituidos pelo menu.
class PropriedadesMeioPoroso {
private:
    double densidadeFaseSolida{};              // kg/m3
    double porosidade{};                       // adimensional
    double tortuosidade{};                     // adimensional
    double permeabilidade{};                   // m2
    double moduloCisalhamento{};               // Pa
    double parametroLame{};                    // Pa
    double moduloBiot{};                       // Pa
    double parametroBiotWillis{};              // adimensional
    double densidadeFluido{};                  // kg/m3
    double viscosidadeFluido{};                // Pa.s
    double comprimentoCaracteristicoViscoso{}; // m
    double expoenteCimentacao{};               // adimensional
    double calorEspecifico{};                  // J/(kg.K)
    double condutividadeTermica{};             // W/(m.K)
    double tempoRelaxacaoTermica{};            // s
    double coeficienteTermicoSolido{};         // Pa/K
    double coeficienteTermicoFluido{};         // Pa/K
    double temperaturaReferencia{};            // K

public:
    static PropriedadesMeioPoroso arenitoSaturadoComAgua() {
        PropriedadesMeioPoroso meio{};
        meio.densidadeFaseSolida = 2644.0;
        meio.porosidade = 0.20;
        meio.tortuosidade = 2.40;
        meio.permeabilidade = 3.60e-13;
        meio.moduloCisalhamento = 7.04e9;
        meio.parametroLame = 10.60e9;
        meio.moduloBiot = 9.70e9;
        meio.parametroBiotWillis = 0.72;
        meio.densidadeFluido = 1000.0;
        meio.viscosidadeFluido = 1.00e-3;
        meio.comprimentoCaracteristicoViscoso = 5.88e-6;
        meio.expoenteCimentacao = 1.0;
        meio.calorEspecifico = 1586.8;
        meio.condutividadeTermica = 2.75;
        meio.tempoRelaxacaoTermica = 1.00e-6;
        meio.coeficienteTermicoSolido = 550560.0;
        meio.coeficienteTermicoFluido = 667360.0;
        meio.temperaturaReferencia = 300.0;
        return meio;
    }

    double getDensidadeFaseSolida() const { return densidadeFaseSolida; }
    void setDensidadeFaseSolida(double valor) { densidadeFaseSolida = valor; }

    double getPorosidade() const { return porosidade; }
    void setPorosidade(double valor) { porosidade = valor; }

    double getTortuosidade() const { return tortuosidade; }
    void setTortuosidade(double valor) { tortuosidade = valor; }

    double getPermeabilidade() const { return permeabilidade; }
    void setPermeabilidade(double valor) { permeabilidade = valor; }

    double getModuloCisalhamento() const { return moduloCisalhamento; }
    void setModuloCisalhamento(double valor) { moduloCisalhamento = valor; }

    double getParametroLame() const { return parametroLame; }
    void setParametroLame(double valor) { parametroLame = valor; }

    double getModuloBiot() const { return moduloBiot; }
    void setModuloBiot(double valor) { moduloBiot = valor; }

    double getParametroBiotWillis() const { return parametroBiotWillis; }
    void setParametroBiotWillis(double valor) { parametroBiotWillis = valor; }

    double getDensidadeFluido() const { return densidadeFluido; }
    void setDensidadeFluido(double valor) { densidadeFluido = valor; }

    double getViscosidadeFluido() const { return viscosidadeFluido; }
    void setViscosidadeFluido(double valor) { viscosidadeFluido = valor; }

    double getComprimentoCaracteristicoViscoso() const {
        return comprimentoCaracteristicoViscoso;
    }
    void setComprimentoCaracteristicoViscoso(double valor) {
        comprimentoCaracteristicoViscoso = valor;
    }

    double getExpoenteCimentacao() const { return expoenteCimentacao; }
    void setExpoenteCimentacao(double valor) { expoenteCimentacao = valor; }

    double getCalorEspecifico() const { return calorEspecifico; }
    void setCalorEspecifico(double valor) { calorEspecifico = valor; }

    double getCondutividadeTermica() const { return condutividadeTermica; }
    void setCondutividadeTermica(double valor) { condutividadeTermica = valor; }

    double getTempoRelaxacaoTermica() const { return tempoRelaxacaoTermica; }
    void setTempoRelaxacaoTermica(double valor) { tempoRelaxacaoTermica = valor; }

    double getCoeficienteTermicoSolido() const { return coeficienteTermicoSolido; }
    void setCoeficienteTermicoSolido(double valor) {
        coeficienteTermicoSolido = valor;
    }

    double getCoeficienteTermicoFluido() const { return coeficienteTermicoFluido; }
    void setCoeficienteTermicoFluido(double valor) {
        coeficienteTermicoFluido = valor;
    }

    double getTemperaturaReferencia() const { return temperaturaReferencia; }
    void setTemperaturaReferencia(double valor) { temperaturaReferencia = valor; }
};
