
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file cranknicolson.hpp
    \brief Crank-Nicolson scheme for time evolution

    $Id$
*/

// $Source$
// $Log$
// Revision 1.4  2001/06/18 10:20:25  nando
// 80 colums enforced
//
// Revision 1.3  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_crank_nicolson_h
#define quantlib_crank_nicolson_h

#include "ql/date.hpp"
#include "ql/FiniteDifferences/identity.hpp"
#include "ql/FiniteDifferences/operatortraits.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        // WARNING: the differential operator D must be linear for
        // this evolver to work!

        /*  Operators must be derived from either TimeConstantOperator or
            TimeDependentOperator.
            They must also implement at least the following interface:

            // copy constructor/assignment
            // if no particular care is required, these two can be
            // omitted. They will be provided by the compiler.
            Operator(const Operator&);
            Operator& operator=(const Operator&);

            // modifiers
            void setTime(Time t);  // those derived from TimeConstantOperator
                                   // might skip this if the compiler allows it.

            // operator interface
            arrayType applyTo(const arrayType&);
            arrayType solveFor(const arrayType&);

            // operator algebra
            Operator operator*(double,const Operator&);
            Operator operator+(const Identity<arrayType>&,const Operator&);
        */

        template <class Operator>
        class CrankNicolson {
            friend class FiniteDifferenceModel<CrankNicolson<Operator> >;
          private:
            // typedefs
            typedef typename OperatorTraits<Operator>::arrayType arrayType;
            typedef Operator operatorType;
            // constructors
            CrankNicolson(const operatorType& D) : D(D), dt(0.0) {}
            void step(arrayType& a, Time t);
            void setStep(Time dt) {
                this->dt = dt;
                explicitPart = Identity<arrayType>()-(dt/2)*D;
                implicitPart = Identity<arrayType>()+(dt/2)*D;
            }
            operatorType D, explicitPart, implicitPart;
            Time dt;
            #if QL_TEMPLATE_METAPROGRAMMING_WORKS
                // a bit of template metaprogramming to relax interface
                // constraints on time-constant operators
                // see T. L. Veldhuizen, "Using C++ Template Metaprograms",
                // C++ Report, Vol 7 No. 4, May 1995
                // http://extreme.indiana.edu/~tveldhui/papers/
                template <int constant>
                class CrankNicolsonTimeSetter {};
                // the following specialization will be instantiated if
                // Operator is derived from TimeConstantOperator
                template<>
                class CrankNicolsonTimeSetter<0> {
                  public:
                    static inline void setTime(Operator& D,
                                               Operator& explicitPart,
                                               Operator& implicitPart,
                                               Time      t,
                                               Time      dt) {}
                };
                // the following specialization will be instantiated if Operator
                // is derived from TimeDependentOperator:
                // only in this case Operator will be required
                // to implement void setTime(Time t)
                template<>
                class CrankNicolsonTimeSetter<1> {
                    typedef typename OperatorTraits<Operator>::arrayType
                        arrayType;
                  public:
                    static inline void setTime(Operator& D,
                                               Operator& explicitPart,
                                               Operator& implicitPart,
                                               Time      t,
                                               Time      dt) {
                        D.setTime(t);
                        explicitPart = Identity<arrayType>()-(dt/2)*D;
                        implicitPart = Identity<arrayType>()+(dt/2)*D;
                    }
                };
            #endif
        };

        // inline definitions

        template <class Operator>
        inline void CrankNicolson<Operator>::step(arrayType& a, Time t) {
            #if QL_TEMPLATE_METAPROGRAMMING_WORKS
                CrankNicolsonTimeSetter<Operator::isTimeDependent>::setTime(D,
                    explicitPart, implicitPart, t, dt);
            #else
                if (Operator::isTimeDependent) {
                    D.setTime(t);
                    explicitPart = Identity<arrayType>()-(dt/2)*D;
                    implicitPart = Identity<arrayType>()+(dt/2)*D;
                }
            #endif
            a = implicitPart.solveFor(explicitPart.applyTo(a));
        }

    }

}


#endif
