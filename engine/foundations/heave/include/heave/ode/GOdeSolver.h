#pragma once

#include "fortress/templates/GTemplates.h"
#include "heave/ode/GOde.h"

namespace rev {

/// @class OdeSolver
/// @brief Contains various solvers for ODEs
class OdeSolver {
public:
    
    /// @brief Using RK4, solve the ODE, given a step size for the independent variable
    /// @see https://math.okstate.edu/people/yqwang/teaching/math4513_fall11/Notes/rungekutta.pdf
    /// @param[in] outResult The output result for the step
    /// @param[in] q The solution to the ODE at the given time
    /// @param[in] s The independent variable value at which the current solution is known
    /// @param[in] ds The step size to advance from the current step
    /// @param[in] odeFunctions The solutions to the ODE types
    template<typename ...OdeFunctionTypes, typename FloatType>
    static void Rk4(
        std::array<FloatType, sizeof...(OdeFunctionTypes)>& newQ, 
        FloatType& newS,
        const std::array<FloatType, sizeof...(OdeFunctionTypes)>& q,
        FloatType s,
        FloatType ds, 
        OdeFunctionTypes... odeFunctions) 
    {
        constexpr Uint32_t NumEquations = sizeof...(OdeFunctionTypes);

        /// Obtain four estimates
        std::array<FloatType, NumEquations> dq1;
        std::array<FloatType, NumEquations> dq2;
        std::array<FloatType, NumEquations> dq3;
        std::array<FloatType, NumEquations> dq4;
        std::array<FloatType, NumEquations> tmpQ; // Intermediate values used to step through

        // First estimate
        std::size_t k = 0;
        (std::invoke(odeFunctions, dq1[k++], s, q), ...); // Fold expression!

        // Second estimate
        k = 0;
        FloatType scaledStep = FloatType(0.5) * ds;
        OdeSolver::SolveForIntermediateValuesRk4(tmpQ, q, scaledStep, dq1);
        FloatType s2 = s + scaledStep;
        (std::invoke(odeFunctions, dq2[k++], s2, tmpQ), ...); // Fold expression!

        // Third estimate
        k = 0;
        OdeSolver::SolveForIntermediateValuesRk4(tmpQ, q, scaledStep, dq2);
        (std::invoke(odeFunctions, dq3[k++], s2, tmpQ), ...); // Fold expression!

        // Fourth estimate
        k = 0;
        scaledStep = ds; // 1.0 * ds for full step size
        OdeSolver::SolveForIntermediateValuesRk4(tmpQ, q, scaledStep, dq3);
        newS = s + ds;
        (std::invoke(odeFunctions, dq4[k++], newS, tmpQ), ...); // Fold expression!

        /// Update the dependent and independent variable values at the new
        /// dependent variable location and store the values in the ODE object arrays.
        constexpr FloatType factor = FloatType(1) / FloatType(6);
        FloatType factorTimeStep = factor * ds;
        for (size_t i = 0; i < NumEquations; ++i) {
            newQ[i] = q[i] + (factorTimeStep * (dq1[i] + 2.0 * dq2[i] + 2.0 * dq3[i] + dq4[i]));
        }
    }


    /// @brief Using RK45, solve the ODE, given an error tolerance
    /// @see https://maths.cnam.fr/IMG/pdf/RungeKuttaFehlbergProof.pdf
    /// @see https://math.okstate.edu/people/yqwang/teaching/math4513_fall11/Notes/rungekutta.pdf
    /// @see https://www.youtube.com/watch?v=6bCBXvsD7gw
    /// @param[in/out] ode the ODE to step through with RK45
    /// @param[out] newQ the result
    /// @param[out] newS the resulting value of the independent variable
    /// @param[out] newDs the next value of the independent variable step size to use
    /// @param[out] outSuccess whether or not the current iteration was within the error tolerance
    /// @param[out] q the current value of the ODE dependent variables
    /// @param[out] s the value of the ODE independent variable
    /// @param[in] tolerance the acceptable relative error tolerance to solve the equaiton with. 1e-6 is a reasonable start
    /// @param[in] ds the step size. Will be modified until the error is under the acceptable tolerance
    /// @return The next step size
    template<typename ...OdeFunctionTypes, typename FloatType>
    static void Rk45(
        std::array<FloatType, sizeof...(OdeFunctionTypes)>& newQ,
        FloatType& newS,
        FloatType& newDs,
        bool& outSuccess, 
        std::array<FloatType, sizeof...(OdeFunctionTypes)>& q,
        FloatType s,
        FloatType tolerance, 
        FloatType ds,
        OdeFunctionTypes... odeFunctions)
    {
        constexpr Uint32_t NumEquations = sizeof...(OdeFunctionTypes);

        /// Obtain five estimates
        std::array<FloatType, NumEquations> dq1;
        std::array<FloatType, NumEquations> dq2;
        std::array<FloatType, NumEquations> dq3;
        std::array<FloatType, NumEquations> dq4;
        std::array<FloatType, NumEquations> dq5;
        std::array<FloatType, NumEquations> dq6;
        std::array<FloatType, NumEquations> q4; // Intermediate values used to step through

        // Get first estimate
        std::size_t k = 0;
        (std::invoke(odeFunctions, dq1[k++], s, q), ...); // Fold expression!

        // Get second estimate
        k = 0;
        OdeSolver::SolveForIntermediateValuesRk45(q4, q, ds, dq1);
        constexpr FloatType stepFactor2 = 0.25;
        FloatType s2 = s + stepFactor2 * ds;
        (std::invoke(odeFunctions, dq2[k++], s2, q4), ...); // Fold expression!

        // Get third estimate
        k = 0;
        OdeSolver::SolveForIntermediateValuesRk45(q4, q, ds, dq1, dq2);
        constexpr FloatType stepFactor3 = 3.0 / 8.0;
        FloatType s3 = s + stepFactor3 * ds;
        (std::invoke(odeFunctions, dq3[k++], s3, q4), ...); // Fold expression!

        // Get fourth estimate
        k = 0;
        OdeSolver::SolveForIntermediateValuesRk45(q4, q, ds, dq1, dq2, dq3);
        constexpr FloatType stepFactor4 = 12.0 / 13.0;
        FloatType s4 = s + stepFactor4 * ds;
        (std::invoke(odeFunctions, dq4[k++], s4, q4), ...); // Fold expression!

        // Get fifth estimate
        k = 0;
        OdeSolver::SolveForIntermediateValuesRk45(q4, q, ds, dq1, dq2, dq3, dq4);
        FloatType s5 = s + ds; // Step factor is 1.0
        (std::invoke(odeFunctions, dq5[k++], s5, q4), ...); // Fold expression!

        // Get sixth estimate
        k = 0;
        OdeSolver::SolveForIntermediateValuesRk45(q4, q, ds, dq1, dq2, dq3, dq4, dq5);
        constexpr FloatType stepFactor6 = 0.5;
        FloatType s6 = s + stepFactor6 * ds;
        (std::invoke(odeFunctions, dq6[k++], s6, q4), ...); // Fold expression!

        /// Obtain the new dependent variables and relative errors
        constexpr std::array<FloatType, 4> factors = {
            25.0 / 216.0,
            1408.0 / 2565.0,
            2197.0 / 4101.0,
            -1.0 / 5.0
        };
        constexpr std::array<FloatType, 5> moreFactors = {
            16.0 / 135.0,
            6656.0 / 12825.0, 
            28561.0 / 56430.0,
            -9.0 / 50.0,
            2.0 / 55.0
        };
        std::array<FloatType, NumEquations> q5;
        FloatType errorMax = 0;
        FloatType inverseTimeStep = (1.0 / ds);
        for (size_t i = 0; i < NumEquations; ++i) {
            // Obtain dependent variable values
            q4[i] = q[i] +
                ds * (factors[0] * dq1[i] + 
                factors[1] * dq3[i] + 
                factors[2] * dq4[i] + 
                factors[3] * dq5[i]); // Fourth order solution
            q5[i] = q[i] +
                ds * (moreFactors[0] * dq1[i] +
                moreFactors[1] * dq3[i] +
                moreFactors[2] * dq4[i] +
                moreFactors[3] * dq5[i] +
                moreFactors[4] * dq6[i]); // Fifth order solution
            errorMax = std::max(errorMax, FloatType(abs(q5[i] - q4[i])));
        }

        /// Update the independent variable
        //errorMax *= inverseTimeStep;
        if (errorMax <= tolerance) {
            // Error is within tolerance, decrease time-step for next time
            newS = s5; // Set step to s + ds
            newQ.swap(q4); // Set to fourth-order solution
            outSuccess = true;
        }
        else {
            outSuccess = false; // Failed to obtain solution within error tolerance
        }
        FloatType stepDelta = 0.84 * pow(tolerance / errorMax, 0.25);
        newDs = stepDelta * ds;
    }


private:
    template<Int32_t NumEquations, typename FloatType>
    friend class Ode;

    template<Int32_t NumEquations, typename FloatType>
    static inline void SolveForIntermediateValuesRk4(
        std::array<FloatType, NumEquations>& outIntermediateValues,
        const std::array<FloatType, NumEquations>& dependentVariables,
        const FloatType& scaledStep,
        const std::array<FloatType, NumEquations>& dependentVariableDeltas)
    {
        for (size_t i = 0; i < dependentVariables.size(); i++) {
            outIntermediateValues[i] =
                dependentVariables[i] +
                scaledStep * dependentVariableDeltas[i];
        }
    }

    /// @brief Obtain intermediate values of dependent variables of an ODE
    /// @note For Runge-Kutta-Fehlberg (RK45) method
    /// @see https://math.okstate.edu/people/yqwang/teaching/math4513_fall11/Notes/rungekutta.pdf
    template<Int32_t NumEquations, typename FloatType, typename ...Args>
    static inline void SolveForIntermediateValuesRk45(
        typename std::array<FloatType, NumEquations>& outIntermediateValues,
        const std::array<FloatType, NumEquations>& dependentVariables,
        const FloatType& stepSize,
        const Args&... dependentVariableDeltas)
    {
        static_assert(are_same_v<std::array<FloatType, NumEquations>, Args...>, "Dependent variables must all be same type");
        constexpr Uint32_t StepIndex = sizeof...(Args);
        std::tuple<const Args&...> argTuple = std::tie(dependentVariableDeltas...);

        if constexpr (StepIndex == 1) {
            constexpr std::array<FloatType, StepIndex> factors = { 0.25 };
            const std::array<FloatType, NumEquations>& dq1 = std::get<0>(argTuple);
            for (size_t i = 0; i < dependentVariables.size(); i++) {
                outIntermediateValues[i] =
                    dependentVariables[i] + stepSize * factors[0] * dq1[i];
            }
        }
        else if constexpr (StepIndex == 2) {
            constexpr std::array<FloatType, StepIndex> factors = {
                3.0 / 32.0,
                9.0 / 32.0 
            };
            const std::array<FloatType, NumEquations>& dq1 = std::get<0>(argTuple);
            const std::array<FloatType, NumEquations>& dq2 = std::get<1>(argTuple);
            for (size_t i = 0; i < dependentVariables.size(); i++) {
                outIntermediateValues[i] =
                    dependentVariables[i] +
                    stepSize * 
                    (factors[0] * dq1[i] +
                    factors[1] * dq2[i]);
            }
        }
        else if constexpr (StepIndex == 3) {
            constexpr std::array<FloatType, StepIndex> factors = {
                1932.0 / 2197.0,
                -7200.0 / 2197.0,
                7296.0 / 2197.0
            };
            const std::array<FloatType, NumEquations>& dq1 = std::get<0>(argTuple);
            const std::array<FloatType, NumEquations>& dq2 = std::get<1>(argTuple);
            const std::array<FloatType, NumEquations>& dq3 = std::get<2>(argTuple);
            for (size_t i = 0; i < dependentVariables.size(); i++) {
                outIntermediateValues[i] =
                    dependentVariables[i] +
                    stepSize *
                    (factors[0] * dq1[i] + 
                    factors[1] * dq2[i] + 
                    factors[2] * dq3[i]);
            }
        }
        else if constexpr (StepIndex == 4) {
            constexpr std::array<FloatType, StepIndex> factors = {
                439.0 / 216.0,
                -8.0,
                3680.0 / 513.0,
                -845.0 / 4104.0
            };
            const std::array<FloatType, NumEquations>& dq1 = std::get<0>(argTuple);
            const std::array<FloatType, NumEquations>& dq2 = std::get<1>(argTuple);
            const std::array<FloatType, NumEquations>& dq3 = std::get<2>(argTuple);
            const std::array<FloatType, NumEquations>& dq4 = std::get<3>(argTuple);
            for (size_t i = 0; i < dependentVariables.size(); i++) {
                outIntermediateValues[i] =
                    dependentVariables[i] +
                    stepSize *
                    (factors[0] * dq1[i] +
                    factors[1] * dq2[i] + 
                    factors[2] * dq3[i] +
                    factors[3] * dq4[i]);
            }
        }
        else if constexpr (StepIndex == 5) {
            constexpr std::array<FloatType, StepIndex> factors = {
                -8.0 / 27.0,
                2.0,
                -3544.0 / 2565.0,
                1859.0 / 4104.0,
                -11.0 / 40.0
            };
            const std::array<FloatType, NumEquations>& dq1 = std::get<0>(argTuple);
            const std::array<FloatType, NumEquations>& dq2 = std::get<1>(argTuple);
            const std::array<FloatType, NumEquations>& dq3 = std::get<2>(argTuple);
            const std::array<FloatType, NumEquations>& dq4 = std::get<3>(argTuple);
            const std::array<FloatType, NumEquations>& dq5 = std::get<4>(argTuple);
            for (size_t i = 0; i < dependentVariables.size(); i++) {
                outIntermediateValues[i] =
                    dependentVariables[i] +
                    stepSize *
                    (factors[0] * dq1[i] +
                    factors[1] * dq2[i] +
                    factors[2] * dq3[i] +
                    factors[3] * dq4[i] +
                    factors[4] * dq5[i]);
            }
        }
        else {
            static_assert(false, "Invalid step index");
        }
    }
};


} /// rev
