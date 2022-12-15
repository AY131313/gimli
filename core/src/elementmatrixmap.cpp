/******************************************************************************
 *   Copyright (C) 2006-2021 by the GIMLi development team                    *
 *   Carsten Rücker carsten@resistivity.net                                   *
 *                                                                            *
 *   Licensed under the Apache License, Version 2.0 (the "License");          *
 *   you may not use this file except in compliance with the License.         *
 *   You may obtain a copy of the License at                                  *
 *                                                                            *
 *       http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                            *
 *   Unless required by applicable law or agreed to in writing, software      *
 *   distributed under the License is distributed on an "AS IS" BASIS,        *
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *   See the License for the specific language governing permissions and      *
 *   limitations under the License.                                           *
 *                                                                            *
 ******************************************************************************/

#include "elementmatrix.h"
#include "elementmatrixmap.h"
#include "shape.h"
#include "meshentities.h"
#include "node.h"
#include "pos.h"
#include "sparsematrix.h"

#include "integration.h"

namespace GIMLI{

void ElementMatrixMap::resize(Index size){
    mats_.resize(size);
    rows_ = this->mats_.size();
}

void ElementMatrixMap::push_back(const ElementMatrix < double > & Ai){
    mats_.push_back(Ai);
    rows_ = this->mats_.size();
}

template < class ValueType >
void _T_integrateLConst(const ElementMatrixMap * self,
                     const ValueType & f, RVector & R, bool neg){
    ASSERT_NON_EMPTY(R)
    for (auto &m : self->mats()){
        if (neg) {
            m.integrate(f, R, -1);
        } else {
            m.integrate(f, R, 1);
        }
    }
}

template < class ValueType >
void _T_integrateLPerCell(const ElementMatrixMap * self,
                        const ValueType & f, RVector & R, bool neg){
    ASSERT_NON_EMPTY(R)
    ASSERT_EQUAL_SIZE(self->mats(), f)

    for (auto &m : self->mats()){
        if (neg) {
            m.integrate(f[m.entity()->id()], R, -1);
        } else {
            m.integrate(f[m.entity()->id()], R, 1);
        }
    }
}

void ElementMatrixMap::fillSparsityPattern(RSparseMatrix & R) const {
<<<<<<< Updated upstream
    // this * this.T -> quadratic (not necessary) symmetric e.g. u*c
//    __M
=======
    // this * this.T -> quadratic symmetric
>>>>>>> Stashed changes
    const ElementMatrixMap & A = *this;

    //__MS(&A, A.dofA(), A.dofB(), R.rows(), R.cols())
    
    Stopwatch sw(true);
    if (R.rows() == A.dof() && R.cols() == A.dofB()){
        // assume R have already a valid pattern
        return ;
    } 
    // maybe count dofs before
    std::vector < std::set< Index > > idxMap(A.dof());
    Index i = 0;
    for (auto &m : A.mats()){
        const IndexArray &a = m.rowIDs();
        const IndexArray &b = m.colIDs();

        for (Index k = 0; k < a.size(); k ++){
            for (Index l = 0; l < b.size(); l ++){
                idxMap.at(a[k]).insert(b[l]);
            }
        }
        i++;
    }
<<<<<<< Updated upstream
    // __MS("pattern A*A.T (idx)", sw.duration(true))
    if (R.rows() > 0 && R.cols() > 0){
        // assume R is valid and the new pattern need to be add
        R.addSparsityPattern(idxMap);
        // __MS("addpattern A*A.T (build)", sw.duration(true))
    } else {
        // R.resize(A.dof(), A.dofB());
        R.buildSparsityPattern(idxMap);
        // __MS("pattern A*A.T (build)", sw.duration(true))
    }
=======
    __MS("pattern A*A.T (idx)", sw.duration(true))
    R.buildSparsityPattern(idxMap);
    __MS("pattern A*A.T (build)", sw.duration(true))
>>>>>>> Stashed changes
    
    // __MS(R.values().size())
}

void ElementMatrixMap::fillSparsityPattern(RSparseMatrix & R,
                                           const ElementMatrixMap & B) const {
    // this * B.T

    // __MS(R.rows(), R.cols(), this->dof(), this->dofB(), B.dof(), B.dofB())
    Stopwatch sw(true);
    const ElementMatrixMap & A = *this;

    if (R.rows() == A.dof() && R.cols() == B.dof()){
        // assume R have already valid pattern
        return ;
    }

    if (A.size() == 1 && A.mats()[0].order() == 0){
        //const_space * B
        THROW_TO_IMPL
    }
    if (B.size() == 1 && B.mats()[0].order() == 0){
        //A * const_space
        THROW_TO_IMPL
    }
    ASSERT_EQUAL_SIZE(A.mats(), B.mats())

    std::vector < std::set< Index > > idxMap(A.dof());

    Index i = 0;
    for (auto &m : A.mats()){
        const IndexArray &a = m.rowIDs();
        const IndexArray &b = B.mats()[i].rowIDs();
        
        for (Index k = 0; k < a.size(); k ++){
            for (Index l = 0; l < b.size(); l ++){
                idxMap.at(a[k]).insert(b[l]);
            }
        }
        i++;
    }
    // __MS("pattern A*B.T (idx)", sw.duration(true))

<<<<<<< Updated upstream
    if (R.rows() > 0 && R.cols() > 0){
        // assume R is valid and the new pattern need to be add
        R.addSparsityPattern(idxMap);
        // __MS("addpattern A*B.T (build)", sw.duration(true))
    } else {
        // R.resize(A.dof(), A.dofB());
        R.buildSparsityPattern(idxMap);
        // __MS("pattern A*B.T (build)", sw.duration(true))
    }
=======
    __MS("pattern A*B.T (idx)", sw.duration(true))
    R.buildSparsityPattern(idxMap);
    __MS("pattern A*B.T (build)", sw.duration(true))
    //     __MS(R.values().size())
>>>>>>> Stashed changes
}

template < class ValueType >
void _T_integrateBLConst(const ElementMatrixMap & A,
                        const ElementMatrixMap & B,
                        const ValueType & f, SparseMatrixBase & R, bool neg){

    // __MS(f)
    // __MS(typeid(f).name())
    // __MS(typeid(R).name())

    if (A.size() == 1 && A.mats()[0].order() == 0){
        //const_space * B
        Index row = A.mats()[0].dofOffset();
        // __MS(row, A.mats()[0].nCoeff())
        for (auto &m : B.mats()){
            if (!m.isIntegrated()){
                log(Error, "B need to be integrated");
            }
            if (m.nCoeff() == 0){
                THROW_TO_IMPL
            }
            for (Index i = 0; i < m.rows(); i ++){
                // __MS(row, m.rowIDs(), m.getVal(i,0), m.cols())
                R.addVal(row, m.rowIDs()[i], m.getVal(i,0));
            }
        }
        return;
    }
    if (B.size() == 1 && B.mats()[0].order() == 0){
        //A * const_space
        Index col = B.mats()[0].dofOffset();
        for (auto &m : A.mats()){
            if (!m.isIntegrated()){
                log(Error, "B need to be integrated");
            }
            if (m.nCoeff() == 0){
                THROW_TO_IMPL
            }
            for (Index i = 0; i < m.rows(); i ++){
                // __MS(col, m.rowIDs(), m.getVal(i,0))
                R.addVal(m.rowIDs()[i], col, m.getVal(i,0));
            }
        }
        return;
    }
// __M
    ASSERT_EQUAL_SIZE(A.mats(), B.mats())
    Index i = 0;
    for (auto &m : A.mats()){
        if (neg){
            m.integrate(B.mats()[i], f, R, -1.0);
        } else {
            m.integrate(B.mats()[i], f, R, 1.0);
        }
        i++;
    }
}

template < class ValueType >
void _T_integrateBLPerCell(const ElementMatrixMap & A,
                          const ElementMatrixMap & B,
                          const ValueType & f, SparseMatrixBase & R, bool neg){

// __M
    ASSERT_EQUAL_SIZE(A.mats(), B.mats())
    ASSERT_EQUAL_SIZE(A, f)
    Index i = 0;
    for (auto &m : A.mats()){
        if (neg){
            m.integrate(B.mats()[i], f[m.entity()->id()], R, -1.0);
        } else {
            m.integrate(B.mats()[i], f[m.entity()->id()], R, 1.0);
        }
        i++;
    }

}

#define DEFINE_INTEGRATE_ELEMENTMAP_L_IMPL(A_TYPE) \
void ElementMatrixMap::integrate(const A_TYPE & f, \
                                 RVector & R, bool neg) const {\
    _T_integrateLConst(this, f, R, neg); \
} \
void ElementMatrixMap::mult(const A_TYPE & f, ElementMatrixMap & ret) const { \
    ret.resize(this->size());\
    ret.setDof(this->dofA(), this->dofB()); \
    Index i = 0; \
    for (auto const &m : this->mats_){ \
        GIMLI::mult(m, f, *ret.pMat(i)); \
        i++; \
    } \
} \

DEFINE_INTEGRATE_ELEMENTMAP_L_IMPL(double)
DEFINE_INTEGRATE_ELEMENTMAP_L_IMPL(Pos)
DEFINE_INTEGRATE_ELEMENTMAP_L_IMPL(RSmallMatrix)
#undef DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL

void ElementMatrixMap::add(const ElementMatrixMap & B,
                           ElementMatrixMap & ret, Index dim, double b) const {
    // __M
    ASSERT_EQUAL_SIZE(this->mats(), B.mats())

    ret.resize(this->size());
    ret.setDof(this->dofA(), this->dofB());

    Index i = 0;
    for (auto const &m: this->mats_){
        ret.pMat(i)->copyFrom(m);
        ret.pMat(i)->add(B.mats()[i], dim, b);
        i++;
    }

    // __MS(this->mats()[0])
    // __MS(B.mats()[0])
    // __MS(*ret.pMat(0))
}


#define DEFINE_INTEGRATE_ELEMENTMAP_L_PERCELL_IMPL(A_TYPE) \
void ElementMatrixMap::integrate(const A_TYPE & f, RVector & R , bool neg) const { \
    _T_integrateLPerCell(this, f, R, neg); \
} \
void ElementMatrixMap::mult(const A_TYPE & f, ElementMatrixMap & ret) const { \
    ret.resize(this->size()); \
    ret.setDof(this->dofA(), this->dofB()); \
    Index i = 0; \
    for (auto const &m : this->mats_){ \
        GIMLI::mult(m, f[m.entity()->id()], *ret.pMat(i)); \
        i++; \
    } \
} \

DEFINE_INTEGRATE_ELEMENTMAP_L_PERCELL_IMPL(RVector)
DEFINE_INTEGRATE_ELEMENTMAP_L_PERCELL_IMPL(PosVector)
DEFINE_INTEGRATE_ELEMENTMAP_L_PERCELL_IMPL(std::vector< RSmallMatrix >)
DEFINE_INTEGRATE_ELEMENTMAP_L_PERCELL_IMPL(std::vector< RVector >)
DEFINE_INTEGRATE_ELEMENTMAP_L_PERCELL_IMPL(std::vector< PosVector >)
DEFINE_INTEGRATE_ELEMENTMAP_L_PERCELL_IMPL(std::vector< std::vector< RSmallMatrix > >)
#undef DEFINE_INTEGRATE_ELEMENTMAP_L_PERCELL_IMPL
// 

#define DEFINE_INTEGRATE_ELEMENTMAP_BL_IMPL(A_TYPE) \
void ElementMatrixMap::integrate(const ElementMatrixMap & B, const A_TYPE & f, \
                                 SparseMatrixBase & R, bool neg) const {\
    if (R.rtti() == GIMLI_SPARSE_CRS_MATRIX_RTTI){\
        this->fillSparsityPattern(*dynamic_cast< RSparseMatrix * >(&R), B);\
    }\
    _T_integrateBLConst(*this, B, f, R, neg); \
}
DEFINE_INTEGRATE_ELEMENTMAP_BL_IMPL(double)
DEFINE_INTEGRATE_ELEMENTMAP_BL_IMPL(Pos)
DEFINE_INTEGRATE_ELEMENTMAP_BL_IMPL(RSmallMatrix)
#undef DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL


#define DEFINE_INTEGRATE_ELEMENTMAP_BL_PERCELL_IMPL(A_TYPE) \
void ElementMatrixMap::integrate(const ElementMatrixMap & B, const A_TYPE & f, \
                                 SparseMatrixBase & R, bool neg) const {\
    if (R.rtti() == GIMLI_SPARSE_CRS_MATRIX_RTTI){\
        this->fillSparsityPattern(*dynamic_cast< RSparseMatrix * >(&R), B);\
    }\
    _T_integrateBLPerCell(*this, B, f, R, neg); \
}
DEFINE_INTEGRATE_ELEMENTMAP_BL_PERCELL_IMPL(RVector)
DEFINE_INTEGRATE_ELEMENTMAP_BL_PERCELL_IMPL(PosVector)
DEFINE_INTEGRATE_ELEMENTMAP_BL_PERCELL_IMPL(std::vector< RSmallMatrix >)
DEFINE_INTEGRATE_ELEMENTMAP_BL_PERCELL_IMPL(std::vector< RVector >)
DEFINE_INTEGRATE_ELEMENTMAP_BL_PERCELL_IMPL(std::vector< PosVector >)
DEFINE_INTEGRATE_ELEMENTMAP_BL_PERCELL_IMPL(std::vector< std::vector< RSmallMatrix > >)
#undef DEFINE_INTEGRATE_ELEMENTMAP_BL_PERCELL_IMPL


#define DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(A_TYPE) \
RVector ElementMatrixMap::integrate(const A_TYPE & f, bool neg) const { \
    Index maxR = 0; \
    for (auto &m : this->mats_){ \
        maxR = max(maxR, max(m.rowIDs()));\
    } \
    RVector R(maxR+1); \
    integrate(f, R, neg); \
    return R; \
} \

DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(double)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(Pos)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(RSmallMatrix)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(RVector)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(PosVector)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(std::vector< RSmallMatrix >)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(std::vector< RVector >)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(std::vector< PosVector >)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(std::vector< std::vector< RSmallMatrix > >)
#undef DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET

#define DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(A_TYPE) \
RSparseMapMatrix ElementMatrixMap::integrate(const ElementMatrixMap & R, \
                                             const A_TYPE & f, bool neg) const{\
    RSparseMapMatrix A(0,0); \
    integrate(R, f, A, neg); \
    return A; \
}
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(double)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(Pos)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(RSmallMatrix)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(RVector)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(PosVector)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(std::vector< RSmallMatrix >)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(std::vector< RVector >)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(std::vector< PosVector >)
DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET(std::vector< std::vector< RSmallMatrix > >)
#undef DEFINE_INTEGRATE_ELEMENTMAP_R_IMPL_RET

void ElementMatrixMap::dot(const ElementMatrixMap & B,
                           ElementMatrixMap & ret) const {

    if (this->size() == 1 && this->mats()[0].order() == 0){
        //** this is: const Space * B
        const auto &A = this->mats()[0];

        // __MS(A)
        ret.resize(B.size());
        ret.setDof(A.nCoeff() + A.dofOffset(), 
                   B.dof());

        Index row = A.dofOffset();
        Index i = 0;
        Index nCoeff = A.nCoeff();

        for (auto &m : B.mats()){
            if (!m.isIntegrated()){
                log(Error, "B need to be integrated");
            }
            if (m.nCoeff() == 0){
                THROW_TO_IMPL
            }

            ret.pMat(i)->copyFrom(m, false);
            ret.pMat(i)->resize(m.cols(), m.rows(), false);
            ret.pMat(i)->setIds(range(row, row + nCoeff), m.rowIDs());

            //!!Replace with: need ret.pMat(i)->setMat(m.mat().T);

            for (Index j=0; j < m.rows(); j++){
                for (Index k=0; k < m.cols(); k++){
                    ret.pMat(i)->setVal(k, j, m(j, k));
                }
            }
            ret.pMat(i)->integrated(true);
            i++;
        }
        return;
    }
    if (B.size() == 1 && B.mats()[0].order() == 0){
        //** this is: A * const Space
       

        ret.resize(this->size());
        ret.setDof(this->dof(), B.mats()[0].nCoeff() + B.mats()[0].dofOffset());

        Index col = B.mats()[0].dofOffset();
        Index i = 0;
        Index nCoeff = B.mats()[0].nCoeff();

        for (auto &m : this->mats()){
            if (!m.isIntegrated()){
                log(Error, "A need to be integrated");
            }
            if (m.nCoeff() == 0){
                THROW_TO_IMPL
            }

            ret.pMat(i)->copyFrom(m, false);
            ret.pMat(i)->setIds(m.rowIDs(), range(col, col+nCoeff));
            ret.pMat(i)->setMat(m.mat());
            ret.pMat(i)->integrated(true);
            i++;
        }
        return;
    }

    const auto &A = *this;
    // __MS(this->size(), B.size())
    ASSERT_EQUAL_SIZE(A, B)
    // __MS(A.size(), A.dofA(), A.dofB(), B.size(), B.dofA(), B.dofB())
    ret.resize(B.size());
    ret.setDof(A.dof(), B.dof());
    
    Index i = 0;
    for (auto &m : this->mats()){
        GIMLI::dot(m, B.mats()[i], *ret.pMat(i));
        i++;
    }
}

template < class ValueType, class RetType >
void _T_assembleFConst(const ElementMatrixMap * self, const ValueType & f,
                     RetType & R, bool neg){
    ASSERT_NON_EMPTY(R)

    // R.clean(); dont clean
    for (auto &m : self->mats()){
        R.add(m, f, neg);
    }
}

template < >
void _T_assembleFConst(const ElementMatrixMap * self, const double & f,
                     SparseMatrixBase & R, bool neg){
    if (R.size() == 0) R.resize(self->dof(), 0);
    ASSERT_NON_EMPTY(R)

    Stopwatch s(true);
    // R.clean(); dont clean
    for (auto &m : self->mats()){
        R.add(m, f, neg);
    }
    // ALLOW_PYTHON_THREADS
    // double b = f;

    // RSparseMatrix &S = dynamic_cast< RSparseMatrix &>(R);
    // ///// TMP hack

    // if (neg == true) {
    //     __MS("inline")
    //     // R.clean(); dont clean
    //     for (auto &m : self->mats()){
    //         m.integrate();

    //         for (Index i = 0, imax = m.rows(); i < imax; i++){
    //             for (Index j = 0, jmax = m.cols(); j < jmax; j++){
    //                 // __MS(A.rowIDs()[i] << " " << A.colIDs()[j] << "  "
    //                 //       << scale << " " << A.getVal(i, j))
    //                 S.addVal(m.rowIDs()[i], m.colIDs()[j], b * m.getVal(i, j));
    //             }
    //         }
    //     }
    // } else {
    //     __MS("method template")
    //     // for (Index i = 0; i < self->size(); i++){
    //     //     S.addS(self->mats()[i], f, neg);
    //     // }

    //     for (auto &m : self->mats()){
    //         S.addS(m, f, neg);
    //         // S.add(m, f, neg);
    //     }
    // }
    // __MS(s.duration(), s.cycles());
}



template < class ValueType, class RetType >
void assembleFPerCellT_(const ElementMatrixMap * self, const ValueType & f, 
                       RetType & R, bool neg){
    ASSERT_NON_EMPTY(R)
    ASSERT_EQUAL_SIZE(self->mats(), f)
    // R.clean(); dont clean
    for (auto &m : self->mats()){
        R.add(m, f[m.entity()->id()], neg);
    }
}

#define DEFINE_ASSEMBLER_L(A_TYPE) \
void ElementMatrixMap::assemble(const A_TYPE & f, RVector & R, bool neg) const { \
    _T_assembleFConst(this, f, R, neg); \
} \
void ElementMatrixMap::assemble(const A_TYPE & f, SparseMatrixBase & R, bool neg) const { \
    if (R.rtti() == GIMLI_SPARSE_CRS_MATRIX_RTTI){\
        this->fillSparsityPattern(*dynamic_cast< RSparseMatrix * >(&R));\
    }\
    _T_assembleFConst(this, f, R, neg); \
} \

DEFINE_ASSEMBLER_L(double)   // const scalar for all cells
DEFINE_ASSEMBLER_L(RSmallMatrix)  // const Matrix for all cells
DEFINE_ASSEMBLER_L(RVector3)  // const Pos for all cells
#undef DEFINE_ASSEMBLER_L

#define DEFINE_ASSEMBLER_B(A_TYPE) \
void ElementMatrixMap::assemble(const A_TYPE & f, RVector & R, bool neg) const { \
    assembleFPerCellT_(this, f, R, neg); \
} \
void ElementMatrixMap::assemble(const A_TYPE & f, SparseMatrixBase & R, bool neg) const { \
    if (R.rtti() == GIMLI_SPARSE_CRS_MATRIX_RTTI){\
        this->fillSparsityPattern(*dynamic_cast< RSparseMatrix * >(&R));\
    }\
    assembleFPerCellT_(this, f, R, neg); \
} \

DEFINE_ASSEMBLER_B(RVector)  // const scalar for each cell
DEFINE_ASSEMBLER_B(std::vector< RSmallMatrix >)// const matrix for each cell
DEFINE_ASSEMBLER_B(std::vector< RVector3 >)  // const Pos for each cell
#undef DEFINE_ASSEMBLER_B

const std::vector< ElementMatrix < double > > & ElementMatrixMap::mats() const{
    return mats_;
}

const std::vector < PosVector > & ElementMatrixMap::quadraturePoints() const {

    if (this->quadrPnts_.size() != this->mats_.size()){
        this->quadrPnts_.clear();

        for (auto &m: this->mats_){
            const PosVector &x(*m.x());
            this->quadrPnts_.push_back(PosVector(x.size()));
            for (Index i = 0; i < x.size(); i ++){
                this->quadrPnts_.back()[i] = m.entity()->shape().xyz(x[i]);
            }
        }
    }
    return this->quadrPnts_;
}

PosVector ElementMatrixMap::entityCenters() const{
    PosVector ret;
    for (auto &m: this->mats_){
        ret.push_back(m.entity()->shape().center());
    }
    return ret;
}

void ElementMatrixMap::add(Index row, const ElementMatrix < double > & Ai){

    rows_ = max(row + 1, rows_);
    cols_ = max(max(Ai.ids()) + 1, cols_);

    mat_.push_back(Ai.mat());
    _ids.push_back(Ai.ids());
    row_.push_back(row);
}

RVector ElementMatrixMap::mult(const RVector & a, const RVector & b,
                               const RVector & m, const RVector & n) const{
    RVector ret(rows_);

/// refactor me with ElementMatrix.mult
    for (Index r = 0; r < row_.size(); r ++ ){
        double s = 0.0;
        const RSmallMatrix & mat = mat_[r];
        const IndexArray & idx = _ids[r];
        for (Index i = 0; i < mat.rows(); i ++) {
            double t = 0;
            for (Index j = 0; j < mat.cols(); j ++) {
                t += mat(i,j) * (a[idx[j]]-b[idx[j]]);
            }
            s += t * (m[idx[i]] - n[idx[i]]);
        }

        ret[row_[r]] += s;
    }

    return ret;
}

RVector ElementMatrixMap::mult(const RVector & a, const RVector & b) const{
    RVector ret(rows_);

    for (Index r = 0; r < row_.size(); r ++ ){
        double s = 0.0;
        const RSmallMatrix & mat = mat_[r];
        const IndexArray & idx = _ids[r];
        for (Index i = 0; i < mat.rows(); i ++) {
            double t = 0;
            for (Index j = 0; j < mat.cols(); j ++) {
                t += mat(i,j) * (a[idx[j]]);
            }
            s += t * (b[idx[i]]);
        }

        ret[row_[r]] += s;
    }

    return ret;
}

void createUMap(const Mesh & mesh, Index order, ElementMatrixMap & ret,
                Index nCoeff, Index dofOffset){


    // don't use cache here // check!
    if (mesh.nodeCount() == 0){
        // empty mesh. this map is for constant space and only contain 1 entry
        ret.resize(1);
        ret.pMat(0)->init(nCoeff, 1, dofOffset);
        ret.pMat(0)->resize(1, 1, false);
        ret.pMat(0)->pMat()->setVal(0, 0, 1.);
        ret.pMat(0)->setIds(range(dofOffset, dofOffset+nCoeff), {0});
        ret.setDof(dofOffset+nCoeff);
        return;
    }

    ret.setDof(mesh.nodeCount()*nCoeff + dofOffset);
    ret.resize(mesh.cellCount());

    for (auto &cell: mesh.cells()){
        ret.pMat(cell->id())->pot(*cell, order, true,
                                   nCoeff, mesh.nodeCount(), dofOffset);
    }
}

ElementMatrixMap createUMap(const Mesh & mesh, Index order,
                            Index nCoeff, Index dofOffset){
    ElementMatrixMap ret;
    createUMap(mesh, order, ret, nCoeff, dofOffset);
    return ret;
}

void createdUMap(const Mesh & mesh, Index order,
                 ElementMatrixMap & ret,
                 bool elastic, bool div, bool kelvin,
                 Index nCoeff, Index dofOffset){
        // don't use cache here // check!

    ret.resize(mesh.cellCount());
    ret.setDof(mesh.nodeCount() * nCoeff + dofOffset,
               mesh.nodeCount() * nCoeff + dofOffset);

    for (auto &cell: mesh.cells()){
        // grad(const MeshEntity & ent, Index order,
        //      bool elastic, bool sum, bool div,
        //      Index nCoeff, Index dof, Index dofOffset,
        //      bool kelvin)
        ret.pMat(cell->id())->grad(*cell, order,
                                   elastic, false, div,
                                    nCoeff, mesh.nodeCount(), dofOffset,
                                    kelvin);
    }
    // __MS(&ret, ret.dofA(), ret.dofB())
}

ElementMatrixMap createdUMap(const Mesh & mesh, Index order,
                             bool elastic, bool div, bool kelvin,
                             Index nCoeff, Index dofOffset){
    ElementMatrixMap ret;
    createdUMap(mesh, order, ret,
                elastic, div, kelvin,
                nCoeff, dofOffset);
    return ret;
}

void createIdentityMap(const Mesh & mesh, Index order,
                       ElementMatrixMap & ret,
                       Index nCoeff, Index dofOffset){

    ret.resize(mesh.cellCount());
    for (auto &cell: mesh.cells()){
        ret.pMat(cell->id())->identity(*cell, order,
                                       nCoeff, mesh.nodeCount(), dofOffset);
    }
    ret.setDof(mesh.nodeCount()*nCoeff + dofOffset);
}

ElementMatrixMap createIdentityMap(const Mesh & mesh, Index order,
                                   Index nCoeff, Index dofOffset){
    ElementMatrixMap ret;
    createIdentityMap(mesh, order, ret, nCoeff, dofOffset);
    return ret;
}

void sym(const ElementMatrixMap & A, ElementMatrixMap & ret){
/*! Return symmetrized copy of A as 0.5*(A + A.T).
ATM. Only for gradients without Voigt or Kelvin notation.
*/
    ret.resize(A.size());
    Index i = 0;
    for (auto &m: A.mats()){
        sym(m, *ret.pMat(i));
        i++;
    }
    // set row ids() .. maybe rename to dofs
    THROW_TO_IMPL
}
ElementMatrixMap sym(const ElementMatrixMap & A){
    ElementMatrixMap ret;
    sym(A, ret);
    return ret;
}

void tr(const ElementMatrixMap & A, RVector & ret){
    THROW_TO_IMPL
}
RVector tr(const ElementMatrixMap & A){
    RVector ret;
    tr(A, ret);
    return ret;
}


} // namespace GIMLI
