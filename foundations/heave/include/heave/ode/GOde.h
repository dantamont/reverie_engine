#pragma once

#include <array>
#include "fortress/numeric/GSizedTypes.h"

namespace rev {

/// @class Ode
/// @brief Represents an ordinary differential equation
template<Int32_t NumEquations, typename FloatType = Float64_t>
class Ode {
private:
    static_assert(NumEquations > 0, "Cannot have zero equations");
public:
    typedef std::array<FloatType, NumEquations> ArrayType;
    typedef FloatType Float;

    /// @brief Return the number of equations for the ODE
    static Uint32_t GetNumEquations() {
        return NumEquations;
    }

    /// @brief Return the independent variable
    FloatType getIndependentVariable() const {
        return m_independentVariable;
    }

    /// @brief Modify the value of the independent variable
    void setIndependentVariable(FloatType value) {
        m_independentVariable = value;
    }

    /// @brief Return the dependent variable at the given index
    FloatType getDependentVariable(Uint32_t index) const {
        return m_dependentVariables[index];
    }

    /// @brief Modify the value of the dependent variable at the given index
    void setDependentVariable(Uint32_t index, FloatType value) {
        m_dependentVariables[index] = value;
    }

    /// @brief Return all of the dependent variables
    const ArrayType& getDependentVariables() const {
        return m_dependentVariables;
    }

protected:
    friend class OdeSolver;

    ArrayType m_dependentVariables; ///< Array of dependent variables
    FloatType m_independentVariable{ FloatType(0.0) }; ///< The independent variable
};


} /// rev
