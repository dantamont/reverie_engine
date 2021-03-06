#ifndef GB_MATRIX_H
#define GB_MATRIX_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <typeinfo>

// Qt
#include <QDebug>

#include "../GConstants.h"
#include "GVector.h"
#include "GQuaternion.h"
#include "../containers/GContainerExtensions.h"

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ShaderProgram;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TypeDefs
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** @class Matrix
    @brief  A class representing a COLUMN MAJOR matrix of any size
	@details Template takes in the numeric type (e.g. float, double), and size of Matrix
    @note Uses std::move call to instantiate with a std::vector, so does not preserve input
*/
template<class D, size_t R, size_t C>
class Matrix {
public:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @name Static
	/// @{
	/// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Constructors and Destructors
    /// @{
    Matrix() {
		checkValidity();
	}
    Matrix(const QJsonValue& json) {
        loadFromJson(json);
    }
    Matrix(D defaultValue) {
		checkValidity();
		initialize(defaultValue);
	}
    Matrix(const Vector<D, R>& column) {
		checkValidity();
		m_mtx[0] = column;
	}
	Matrix(const std::array<Vector<D, R>, C>& columns) {
		checkValidity();
		m_mtx = columns;
	}
    Matrix(const std::array<D, R*C>& data) {
        checkValidity();
        memcpy(m_mtx.data(), data.data(), sizeof(D)*R*C);
    }
	Matrix(const std::vector<Vector<D, R>>& columns) {
		checkValidity();
		std::copy(columns.begin(), columns.begin() + columns.size(), m_mtx.begin());
	}
    Matrix(const D* arrayPtr) {
        checkValidity();
        memcpy(m_mtx.data(), arrayPtr, sizeof(D)*R*C);
    } 
	Matrix(const std::vector<std::vector<D>>& columns) {
		checkValidity(columns);

		for (unsigned int i = 0; i < columns.size(); i++){
			m_mtx[i] = columns[i];
		}
	}

    ~Matrix() {}
    /// @}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @name Operators
    /// @{
    inline Matrix& operator= (const Matrix<D, R, C> &source) {
		if (this == &source) return *this;

		m_mtx = source.m_mtx;

		return *this;
	}
    inline Matrix& operator= (const std::array<D, R*C> &source) {
        memcpy(m_mtx.data(), source.data(), sizeof(D)*R*C);
        return *this;
    }
    //-----------------------------------------------------------------------------------------------------------------
    inline const D& operator()(int row, int column) const {
        assert(row >= 0 && row < R && column >= 0 && column < C);
        return m_mtx.at(column).at(row);
    }
    //-----------------------------------------------------------------------------------------------------------------
    inline D& operator()(int row, int column) {
        assert(row >= 0 && row < R && column >= 0 && column < C);
        return m_mtx[column][row];
    }

	//-----------------------------------------------------------------------------------------------------------------
    inline D& operator[] (int index) {
		int nRows = numRows();
		int rowIndex = index % nRows;
		int colIndex = (index - rowIndex) / nRows;

		return m_mtx[colIndex][rowIndex];
	}
	//-----------------------------------------------------------------------------------------------------------------
    inline const Vector<D, R> operator* (const Vector<D, C>& source) const {
#ifdef LINALG_USE_EIGEN  
        Vector<D, R> vec = Vector<D, R>::EmptyVector();
        //vec.map() = (*this * Matrix<D, C, 1>(source)).getMap();
        vec.matrixMap() = getMap() * source.getMatrixMap();
        return vec;
#else
		Matrix<D, R, 1> product = *this * Matrix<D, C, 1>(source);
		return Vector<D, R>(product.getColumn(0));
#endif
	}
	//-----------------------------------------------------------------------------------------------------------------
    inline bool operator==(const Matrix<D, R, C>& source) const {
#ifdef LINALG_USE_EIGEN  
        return getMap().isApprox(source.getMap());
#else
		bool isEqual = true;
		for (unsigned int i = 0; i < m_mtx.size(); i++) {
			isEqual &= (getColumn(i) == source.getColumn(i));
		}
		isEqual &= (numRows() == source.numRows()) && (numColumns() == source.numColumns());
		return isEqual;
#endif
	}
	//-----------------------------------------------------------------------------------------------------------------
    inline bool operator!=(const Matrix<D, R, C>& source) const {
#ifdef LINALG_USE_EIGEN  
        return !operator==(source);
#else
		bool sameSize = (numRows() == source.numRows()) && (numColumns() == source.numColumns());
		if (!sameSize) return true;
		for (unsigned int i = 0; i < m_mtx.size(); i++) {
			if (getColumn(i) != source.getColumn(i)) {
				return true;
			}
		}
		return false;
#endif
	}
	//-----------------------------------------------------------------------------------------------------------------
    inline Matrix<D, R, C>& operator*= (D scale) {
		*this = *this * scale;
		return *this;
	}
	//-----------------------------------------------------------------------------------------------------------------
    inline Matrix<D, R, C>& operator+= (const Matrix<D, R, C> &source) {
		*this = *this + source;
		return *this;
	}
	//-----------------------------------------------------------------------------------------------------------------
    inline Matrix<D, R, C>& operator+= (D scale) {
		*this = *this + Matrix<D, R, C>(scale);

		return *this;
	}
	//-----------------------------------------------------------------------------------------------------------------
    inline Matrix<D, R, C>& operator-= (const Matrix<D, R, C> &source) {
		*this = *this - source;
		return *this;
	}
	//-----------------------------------------------------------------------------------------------------------------
    inline Matrix<D, R, C>& operator-= (D scale) {
		*this = *this - Matrix<D, R, C>(scale);
		return *this;
	}
	//-----------------------------------------------------------------------------------------------------------------
    /// @brief Implicit conversion to QString for use with qDebug
    operator QString() const {
		QString string;
		string += "Matrix{\n";
		for (auto& v : m_mtx) {
			string += "{";
			for (auto& e : v) {
				string += QString::number(e);
				string += ", ";
			}
            string.chop(2);
			string += "}\n";
		}
		string += "}\n";

		return string;
	}
	//-----------------------------------------------------------------------------------------------------------------
	const Matrix<D, R, C> operator-() {
		return *this * -1.0;
	}
	//-----------------------------------------------------------------------------------------------------------------
	friend const Matrix<D, R, C> operator*(const Matrix<D, R, C>& matrix, D factor) {
#ifdef LINALG_USE_EIGEN  
        Matrix<D, R, C> outMatrix;
        outMatrix.map()= matrix.getMap() * factor;
#else
        Matrix<D, R, C> outMatrix;
		D val;
		for (int j = 0; j < matrix.numColumns(); j++) {
			for (int i = 0; i < matrix.numRows(); i++) {
				val = matrix.at(i, j) * factor;
                outMatrix.at(i, j) = val;
			}
		}
#endif
        return outMatrix;
	}
	//-----------------------------------------------------------------------------------------------------------------
    friend const Matrix<D, R, C> operator/(const Matrix<D, R, C>&  matrix, D divisor) {
#ifdef LINALG_USE_EIGEN  
        Matrix<D, R, C> outMatrix;
        outMatrix.map() = matrix.getMap() / divisor;
#else
        Matrix<D, R, C> outMatrix;
		D val;
		for (int j = 0; j < matrix.numColumns(); j++) {
			for (int i = 0; i < matrix.numRows(); i++) {
				val = matrix.at(i, j) / divisor;
                outMatrix.at(i, j) = val;
			}
		}
#endif
		return outMatrix;
	}
	//-----------------------------------------------------------------------------------------------------------------
	friend Matrix<D, R, C> operator+(const Matrix<D, R, C>& m1, const Matrix<D, R, C>& m2) {
#ifdef LINALG_USE_EIGEN  
        Matrix<D, R, C> outMatrix;
        outMatrix.map() = m1.getMap() + m2.getMap();
#else
        Matrix<D, R, C> outMatrix;
		D val;
		for (int j = 0; j < m1.numColumns(); j++) {
			for (int i = 0; i < m1.numRows(); i++) {
				val = m1.at(i, j) + m2.at(i, j);
                outMatrix.at(i, j) = val;
			}
		}
#endif
		return outMatrix;

	}
	//-----------------------------------------------------------------------------------------------------------------
	friend Matrix<D, R, C> operator-(const Matrix<D, R, C>& m1, const Matrix<D, R, C>& m2) {
#ifdef LINALG_USE_EIGEN  
        Matrix<D, R, C> outMatrix;
        outMatrix.map() = m1.getMap() - m2.getMap();
#else
        Matrix<D, R, C> outMatrix;
		D val;
		for (int j = 0; j < m1.numColumns(); j++) {
			for (int i = 0; i < m1.numRows(); i++) {
				val = m1.at(i, j) - m2.at(i, j);
                outMatrix.at(i, j) = val;
			}
        }
#endif
		return outMatrix;
	}
	//-----------------------------------------------------------------------------------------------------------------
	template<size_t CC>
	friend Matrix<D, R, CC> operator*(const Matrix<D, R, C>& m1, const Matrix<D, C, CC>& m2) {

#ifdef LINALG_USE_EIGEN  
        Matrix<D, R, CC> outMatrix;
        outMatrix.map() = m1.getMap() * m2.getMap();

        //Eigen::Map<const Eigen::Matrix<D, R, C>> map1(m1.getData());
        //Eigen::Map<const Eigen::Matrix<D, C, CC>> map2(m2.getData());
        //Eigen::Matrix<D, R, CC> outMap = map1 * map2;
        //Matrix<D, R, CC> outMatrix(outMap.data());
        return outMatrix;
#else
        int nCols = m1.numColumns();
        int nCols2 = m2.numColumns();
        int nRows = m1.numRows();

        Matrix<D, R, CC> outMatrix;
        D val;
        for (int j = 0; j < nCols2; j++) {
            for (int i = 0; i < nRows; i++) {
                val = 0;
                for (int k = 0; k < nCols; k++) {
                    val += m1.at(i, k) * m2.at(k, j);
                }
                outMatrix.at(i, j) = val;
    }
        }
        return outMatrix;
#endif
	}
	//-----------------------------------------------------------------------------------------------------------------
	friend Matrix<D, R, C> operator*(D factor, const Matrix<D, R, C>& matrix) {
		return matrix * factor;
	}
    /// @}

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////-
    /// @name Public Methods
    /// @{

    /// @brief Convert to and from Json:
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const {
        QJsonArray matrixArray;
        matrixArray.append("m");
        for (const Vector<D, R>& col : m_mtx) {
            matrixArray.append(col.asJson());
        }
        return matrixArray;
    }
    void loadFromJson(const QJsonValue& json) {
        const QJsonArray& array = json.toArray();
        int numCols = array.size(); 
        int numRows = array.at(1).toArray().size();
        for (int col = 1; col < numCols; col++) { // start at one due to "m" string entry
            for (int row = 0; row < numRows; row++) {
                m_mtx[col-1][row] = D(array.at(col).toArray().at(row).toDouble());
            }
        }
    }

    /// @property Element
    /// @brief Element at the given row and column indices
    inline const D& at(size_t row, size_t column) const { return m_mtx.at(column).at(row); }
    inline D& at(size_t row, size_t column) { return m_mtx.at(column).at(row); }

    /// @brief Number of rows in the matrix
    inline int numRows() const{ return m_mtx.at(0).size(); }

    /// @brief Number of columns in the matrix
    inline int numColumns() const { return m_mtx.size(); }

    /// @brief Whether the matrix is square or not
    inline bool isSquare() const { return numRows() == numColumns(); }

    /// @brief Obtain column at the given index
    inline const Vector<D, R>& getColumn(int index) const { return m_mtx.at(index); }
    inline Vector<D, R>& column(int index) { return m_mtx.at(index); }

    /// @brief Obtain row at the given index
    inline Vector<D, C> getRow(int index) const { 
        Vector<D, C> row = Vector<D, C>::EmptyVector();
        for (size_t i = 0; i < C; i++) {
            row[i] = m_mtx.at(i).at(index);
        }
        return row;
    }

	//-----------------------------------------------------------------------------------------------------------------
    /// @brief Return a transposed version of this matrix
    const Matrix<D, C, R> transposed() const {
#ifdef LINALG_USE_EIGEN
        Matrix<D, C, R> outMatrix;
        outMatrix.map() = getMap().transpose();
#else
		std::vector<std::vector<D>> matrixData;
		D val;
		for (int i = 0; i < numRows(); i++) {
			matrixData.push_back(std::vector<D>());
			for (int j = 0; j < numColumns(); j++) {
				val = at(i, j);
				matrixData.back().push_back(val);
			}
		}
		auto outMatrix = Matrix<D, C, R>(matrixData);
#endif
        return outMatrix;
	}

	//-----------------------------------------------------------------------------------------------------------------
    /// @brief return internal array
    inline const std::array<Vector<D, R>, C>& getArray() const { return m_mtx; }
    inline std::array<Vector<D, R>, C>& array() { return m_mtx; }

    inline const D* getData() const{
        return m_mtx.at(0).getArray().data();
    }
    inline D* data() {
        return m_mtx.at(0).array().data();
    }

#ifdef LINALG_USE_EIGEN
    inline Eigen::Map<const Eigen::Matrix<D, R, C>> getMap() const {
        return Eigen::Map<const Eigen::Matrix<D, R, C>>(getData());
    }
    inline Eigen::Map<Eigen::Matrix<D, R, C>> map() {
        return Eigen::Map<Eigen::Matrix<D, R, C>>(data());
    }
#endif

	//-----------------------------------------------------------------------------------------------------------------
    /// @brief normalize the columns of the matrix
    inline void normalizeColumns() {
		for (auto& column : m_mtx) {
			column.normalize();
		}
	}
	//-----------------------------------------------------------------------------------------------------------------
    /// @brief Returns whether or not the two given matrices contain the same column vectors
    /// @details This match is irrespective of column order
    inline bool columnsMatch(const Matrix& source) {
		for (const auto& column : m_mtx) {
			auto iT = std::find_if(source.m_mtx.begin(), source.m_mtx.end(), [&](const auto& c) {
				return c == column;
			});
			if (iT == source.m_mtx.end()) return false;
		}
		return true;
	}

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Get as std::vector
    inline std::vector<std::vector<D>> asStdVector() const {
        return std::vector<std::vector<D>>(m_mtx.begin(), m_mtx.end());
    }

    /// @}

protected:
	/// @brief Since the matrix class is templated, it needs to be friends with all possible templates
	template<class D, size_t RR, size_t CC> 
	friend class Matrix;

	//-----------------------------------------------------------------------------------------------------------------
	/// @brief Check static assertions for validity of matrix
	void checkValidity() {
		static_assert(C != 0, "Error, null-sized vector cannot be used to initialize a matrix");
		static_assert(R != 0, "Error, empty columns cannot be used to initialize a matrix");
	}
	//-----------------------------------------------------------------------------------------------------------------
	void checkValidity(const std::vector<std::vector<D>>& columns) {
		checkValidity();
		if (R != columns.at(0).size()) {
			throw("Error, input vector size is not consistent with matrix dimensions");
		}
		if (C != columns.size()) {
			throw("Error, input vector size is not consistent with matrix dimensions");
		}
	}
	//-----------------------------------------------------------------------------------------------------------------
    /// @brief Multiply matrix by an array 
    /// @details Size of array is assumed to match the matrix
    const Vector<D, C> operator* (const std::vector<D>& vec) const {
		//std::vector<D> vec(sourceArray, sourceArray + numColumns());

		return *this * Vector<D, C>(vec);
	}
	//-----------------------------------------------------------------------------------------------------------------
    /// @brief checkValidity matrix of the given dimensions
    void initialize() {
		checkValidity();

		// Initialize to identity if square, otherwise zero-out
		m_mtx = std::array<Vector<D, R>, C>();
		m_mtx.fill(Vector<D, R>(std::vector<D>(R, 0.0)));
		if (isSquare()) {
			for (int i = 0; i < C; i++) {
				m_mtx[i][i] = 1;
			}
		}
	}
	//-----------------------------------------------------------------------------------------------------------------
    /// @brief checkValidity matrix of the given dimensions with the given value for all elements
    void initialize(int defaultValue) {
		checkValidity();

		m_mtx = std::array<Vector<D, R>, C>();
		m_mtx.fill(Vector<D, R>(std::vector<D>(R, defaultValue)));
	}

    /// @brief Vector of columns representing the matrix
    std::array<Vector<D, R>, C> m_mtx;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class Square matrix
    @brief A class representing a square matrix
*/
template<class D, size_t N>
class SquareMatrix: public Matrix<D, N, N> {
public:
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////-
    /// @name Static
    /// @{

    static const SquareMatrix<D, N>& Identity() {
        static const SquareMatrix<D, N> identity;
        return identity;
    }

    static SquareMatrix<D, N> EmptyMatrix() {
        return SquareMatrix<D, N>(false);
    }

    static SquareMatrix<D, N>* EmptyMatrixPointer() {
        return new SquareMatrix<D, N>(false);
    }

    /// @}

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////-
    /// @name Constructors and Destructors
    /// @{
    SquareMatrix() : Matrix() {
		setToIdentity();
	}
    SquareMatrix(const std::vector<std::vector<D>>& columns) :
		Matrix(columns)
	{
		if (!isSquare()) {
			throw("Error, matrix is not symmetric");
		}
	}
    SquareMatrix(const D* arrayPtr): 
        Matrix(arrayPtr)
    {
    }
    SquareMatrix(const std::vector<Vector<D, N>>& columns) :
		Matrix(columns)
	{
		if (!isSquare()) {
			throw("Error, matrix is not symmetric");
		}
	}    
    SquareMatrix(const Matrix<D, N, N>& mtx) : 
        Matrix(mtx.getArray()) {
    }
    template<size_t M>
    SquareMatrix(const Matrix<D, M, M>& mtx) : Matrix(){
        checkValidity();
        setToIdentity();
        constexpr size_t size = std::min(M, N);
        for (unsigned int i = 0; i < size; i++) {
            m_mtx[i] = Vector<D, N>(mtx.getColumn(i));
        }
    }
    ~SquareMatrix() {}
    /// @}


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////-
    /// @name Operators
    /// @{
    inline SquareMatrix<D, N>& operator= (const SquareMatrix<D, N> &source) {
        if (this == &source) return *this;

        m_mtx = source.m_mtx;

        return *this;
    }
    inline SquareMatrix<D, N>& operator= (const std::array<D, N*N> &source) {
        memcpy(m_mtx.data(), source.data(), sizeof(D)*N*N);
        return *this;
    }
    //-----------------------------------------------------------------------------------------------------------------
    inline SquareMatrix<D, N>& operator*= (D scale) {
        Matrix<D, N, N>::operator*=(scale);
        return *this;
    }
    //-----------------------------------------------------------------------------------------------------------------
    inline SquareMatrix<D, N>& operator+= (const SquareMatrix<D, N> &source) {
        Matrix<D, N, N>::operator+=(source);
        return *this;
    }
    //-----------------------------------------------------------------------------------------------------------------
    inline SquareMatrix<D, N>& operator+= (D scale) {
        Matrix<D, N, N>::operator+=(scale);
        return *this;
    }
    //-----------------------------------------------------------------------------------------------------------------
    inline SquareMatrix<D, N>& operator-= (const SquareMatrix<D, N> &source) {
        Matrix<D, N, N>::operator-=(source);
        return *this;
    }
    //-----------------------------------------------------------------------------------------------------------------
    inline SquareMatrix<D, N>& operator-= (D scale) {
        Matrix<D, N, N>::operator-=(scale);
        return *this;
    }
    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Cannot self-multiply for matricies of arbitrary dimensions, so this goes here
    inline SquareMatrix<D, N>& operator*= (const SquareMatrix<D, N> &source) {
#ifdef LINALG_USE_EIGEN
        map() *= source.getMap();
        return *this;
#else
        *this = *this * source;
        return *this;
#endif
    }

    /// @}

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////-
    /// @name Public Methods
    /// @{

    /// @brief Return whether or not the matrix is an identity matrix
    inline bool isIdentity(float threshold = 1e-6) const {
#ifdef LINALG_USE_EIGEN  
        return getMap().isIdentity(threshold);
#else
        return qFuzzyCompare(*this, SquareMatrix<D, N>());
#endif
    }

    /// @brief Primarily for multiplying Matrix4x4 by a point
    inline const Vector<D, N - 1> multPoint(const Vector<D, N - 1>& source) const {
        static_assert(N == 4, "Only valid for 4x4 Matrix");
        Vector<D, N> sourceAppended(source, 1);
        Vector<D, N> result = Vector<D, N>::EmptyVector();
        result.matrixMap() = getMap() * sourceAppended.getMatrixMap();
        //Matrix<D, N, 1> product = *this * Matrix<D, N, 1>(sourceAppended);
        //return Vector<D, N - 1>(product.getColumn(0));
        return Vector<D, N-1>(result);
    }

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Set this matrix to the identity matrix
    inline void setToIdentity() {
		for (int j = 0; j < N; j++) {
			for (int i = 0; i < N; i++) {
				if (i == j) {
					m_mtx[i][j] = 1;
				}
				else {
					m_mtx[i][j] = 0;
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------------------------------------
    /// @brief Return the diagonal of this matrix
    inline Vector<D, N> diagonal() const {
        Vector<D, N> diagonal = Vector<D, N>::EmptyVector();
		for (int j = 0; j < N; j++) {
			diagonal[j] = at(j, j);
		}

		return diagonal;
	}
	//-----------------------------------------------------------------------------------------------------------------
    /// @return the size of the matrix
    inline int size() const { return numRows(); }
	//-----------------------------------------------------------------------------------------------------------------
	/// @brief Transpose multiply
	/// @details Multiply the transpose of this matrix by another matrix.  Is more performant than a transpose then multiply
	template<size_t CC>
    inline const Matrix<D, N, CC> transposeMultiply(const Matrix<D, N, CC>& source) const {
		//int nRows = numRows();
	 //   int nSourceCols = source.numColumns();
#ifdef LINALG_USE_EIGEN  
        Matrix<D, N, CC> outMatrix;
        outMatrix.map() = getMap().transpose().lazyProduct(source.getMap());
#else
		std::vector<std::vector<D>> matrixData;
		D val;
		for (int j = 0; j < source.numColumns(); j++) {
			matrixData.push_back(std::vector<D>());
			for (int i = 0; i < numColumns(); i++) {
				val = 0;
				for (int k = 0; k < numRows(); k++) {
					val += at(k, i) * source.at(k, j);
				}
				matrixData.back().push_back(val);
			}
		}

		auto outMatrix = Matrix<D, N, CC>(matrixData);
#endif
		return outMatrix;
	}
    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Adds a scale component to the matrix
    template<size_t NN>
    inline void addScale(const Vector<D, NN>& scale) {
        constexpr size_t minDim = std::min(N, NN);
        for (size_t i = 0; i < minDim; i++) {
            m_mtx[i] *= scale.at(i);
        }
    }

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Adds a generic rotation to this matrix
    /// @details Given angle must be in radians
    inline void addRotate(const Vector<D, 3>& axis, float angle) {
        static_assert(N==4, "N must be 4");
        rev::Quaternion quaternion = rev::Quaternion::fromAxisAngle(axis.x(), axis.y(), axis.z(), angle);
        auto mtx = quaternion.toRotationMatrix4x4();

        for (int j = 0; j < size(); j++) {
            for (int i = 0; i < size(); i++) {
                (*this)(i, j) = (D)mtx(i, j);
            }
        }
    }
    

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Adds a rotation about the x-axis to this matrix
    /// @details Given angle must be in radians
    inline void addRotateX(double angle) {
        static_assert(N == 4, "Error, size of matrix is invalid to add rotation");

        double sinTheta = sin(angle);
        double cosTheta = cos(angle);

        double a01 = (*this)(0, 1);
        double a11 = (*this)(1, 1);
        double a21 = (*this)(2, 1);
        double a31 = (*this)(3, 1);
        double a02 = (*this)(0, 2);
        double a12 = (*this)(1, 2);
        double a22 = (*this)(2, 2);
        double a32 = (*this)(3, 2);

        (*this)[4] = a01 * cosTheta + a02 * sinTheta;
        (*this)[5] = a11 * cosTheta + a12 * sinTheta;
        (*this)[6] = a21 * cosTheta + a22 * sinTheta;
        (*this)[7] = a31 * cosTheta + a32 * sinTheta;

        (*this)[8] = a01 * -sinTheta + a02 * cosTheta;
        (*this)[9] = a11 * -sinTheta + a12 * cosTheta;
        (*this)[10] = a21 * -sinTheta + a22 * cosTheta;
        (*this)[11] = a31 * -sinTheta + a32 * cosTheta;
    }

    /// @brief Adds a rotation about the y-axis to this matrix
    /// @details Given angle must be in radians
    inline void addRotateY(double angle) {
        static_assert(N == 4, "Error, size of matrix is invalidi to add rotation");

        double sinTheta = sin(angle);
        double cosTheta = cos(angle);

        double a00 = (*this)(0, 0);
        double a10 = (*this)(1, 0);
        double a20 = (*this)(2, 0);
        double a30 = (*this)(3, 0);
        double a02 = (*this)(0, 2);
        double a12 = (*this)(1, 2);
        double a22 = (*this)(2, 2);
        double a32 = (*this)(3, 2);

        (*this)[0] = a00 * cosTheta + a02 * -sinTheta;
        (*this)[1] = a10 * cosTheta + a12 * -sinTheta;
        (*this)[2] = a20 * cosTheta + a22 * -sinTheta;
        (*this)[3] = a30 * cosTheta + a32 * -sinTheta;

        (*this)[8] = a00 * sinTheta + a02 * cosTheta;
        (*this)[9] = a10 * sinTheta + a12 * cosTheta;
        (*this)[10] = a20 * sinTheta + a22 * cosTheta;
        (*this)[11] = a30 * sinTheta + a32 * cosTheta;
    }

    /// @brief Adds a rotation about the z-axis to this matrix
    /// @details Given angle must be in radians
    inline void addRotateZ(double angle) {
        static_assert(N == 4, "Error, size of matrix is invalidi to add rotation");

        double sinTheta = sin(angle);
        double cosTheta = cos(angle);

        double a00 = (*this)(0, 0);
        double a10 = (*this)(1, 0);
        double a20 = (*this)(2, 0);
        double a30 = (*this)(3, 0);
        double a01 = (*this)(0, 1);
        double a11 = (*this)(1, 1);
        double a21 = (*this)(2, 1);
        double a31 = (*this)(3, 1);

        (*this)[0] = a00 * cosTheta + a01 * sinTheta;
        (*this)[1] = a10 * cosTheta + a11 * sinTheta;
        (*this)[2] = a20 * cosTheta + a21 * sinTheta;
        (*this)[3] = a30 * cosTheta + a31 * sinTheta;

        (*this)[4] = a00 * -sinTheta + a01 * cosTheta;
        (*this)[5] = a10 * -sinTheta + a11 * cosTheta;
        (*this)[6] = a20 * -sinTheta + a21 * cosTheta;
        (*this)[7] = a30 * -sinTheta + a31 * cosTheta;
    }

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Set translation component of the matrix
    inline Vector<D, 3> getTranslationVector() const {
        static_assert(N == 4, "Error, size of matrix is invalid to obtain translation");

        const Vector<D, N>& translationColumn = m_mtx.at(3);

        Vector<D, 3> translation = Vector<D, 3>::EmptyVector();
        memcpy(translation.data(), translationColumn.getData(), sizeof(D)*3);

        return translation;
    }

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Set translation component of the matrix
    inline SquareMatrix<D, N> getTranslationMatrix() const {
        static_assert(N == 4, "Error, size of matrix is invalid to obtain translation");

        SquareMatrix<D, N> translation = SquareMatrix<D, N>::EmptyMatrix();
        translation.setTranslation(getTranslationVector());

        return translation;
    }

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Set translation component of the matrix
    inline void setTranslation(const Vector<D, 3>& translation) {
        static_assert(N == 4, "Error, size of matrix is invalid to add translation");

        (*this)[12] = translation.x();
        (*this)[13] = translation.y();
        (*this)[14] = translation.z();
    }

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Converts to a matrix of floats
    inline SquareMatrix<float, N> toFloatMatrix() const {
        //if (typeid(D).hash_code() == typeid(float).hash_code()) { return *this; }
#ifdef LINALG_USE_EIGEN  
        Eigen::Map<const Eigen::Matrix<D, N, N>> map(getData());
        const Eigen::Matrix<float, N, N> mat = map.cast<float>();
        SquareMatrix<float, N> floatMtx(mat.data());
        return floatMtx;
#else
        std::vector<std::vector<float>> floatMtx;
        float val;
        for (int j = 0; j < numColumns(); j++) {
            floatMtx.push_back(std::vector<float>());
            for (int i = 0; i < numRows(); i++) {
                val = at(i, j);
                floatMtx.back().push_back(val);
            }
        }
        return SquareMatrix<float, N>(floatMtx);
#endif
    }

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Converts to a matrix of reals
    inline SquareMatrix<real_g, N> toRealMatrix() const{
        //if (typeid(D).hash_code() == typeid(real_g).hash_code()) { return *this; }
#ifdef LINALG_USE_EIGEN  
        Eigen::Map<const Eigen::Matrix<D, N, N>> map(getData());
        const Eigen::Matrix<real_g, N, N> mat = map.cast<real_g>();
        SquareMatrix<real_g, N> realMtx(mat.data());
        return realMtx;
#else
        std::vector<std::vector<real_g>> realMtx;
        float val;
        for (int j = 0; j < numColumns(); j++) {
            realMtx.push_back(std::vector<real_g>());
            for (int i = 0; i < numRows(); i++) {
                val = at(i, j);
                realMtx.back().push_back(val);
            }
        }
        return SquareMatrix<real_g, N>(realMtx);
#endif
    }
    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Converts to a matrix of doubles
    inline SquareMatrix<double, N> toDoubleMatrix() const {
        //if (typeid(D).hash_code() == typeid(real_g).hash_code()) { return *this; }
#ifdef LINALG_USE_EIGEN  
        Eigen::Map<const Eigen::Matrix<D, N, N>> map(getData());
        const Eigen::Matrix<double, N, N> mat = map.cast<double>();
        SquareMatrix<double, N> doubleMtx(mat.data());
        return doubleMtx;
#else
        std::vector<std::vector<double>> doubleMtx;
        float val;
        for (int j = 0; j < numColumns(); j++) {
            doubleMtx.push_back(std::vector<double>());
            for (int i = 0; i < numRows(); i++) {
                val = at(i, j);
                doubleMtx.back().push_back(val);
            }
        }
        return SquareMatrix<double, N>(doubleMtx);
#endif
    }
    /// @}

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////-
    /// @name Eigenvalue Decomposition
    /// @brief Methods for performing the eigenvalue decomposition of a matrix
    /// @{
    static void QR_Decomposition(const SquareMatrix<D, N>& matrix,
		SquareMatrix<D, N>& QMatrix,
		SquareMatrix<D, N>& RMatrix) {
		int nDim = matrix.size();
		Vector<D, N> u;
		Vector<D, N> e_j;
		Vector<D, N> currentColumn;
		std::vector<Vector<D, N>> uVec;
		std::vector<Vector<D, N>> eVec; // columns of Q matrix
		for (int k = 0; k < nDim; k++) {
			currentColumn = matrix.getColumn(k);
			u = currentColumn;
			D magnitude;
			for (int j = 0; j < k; j++) {
				e_j = eVec[j];
				magnitude = currentColumn.dot(e_j);
				u -= magnitude * e_j;
			}
			Vec::EmplaceBack(uVec, u);
			Vec::EmplaceBack(eVec, u.normalized());
		}
		QMatrix = SquareMatrix<D, N>(eVec);
		RMatrix = QMatrix.transposed() * matrix;
	}
	//-----------------------------------------------------------------------------------------------------------------
    static void toUpperTriangular(const SquareMatrix<D, N>& matrix,
        SquareMatrix<D, N>& triangularMatrix,
        SquareMatrix<D, N>& SMatrix,
        double tolerance) {
		int count = 0;
		double diff = 1;
		SquareMatrix<D, N> prevA, A, Q, R, S;
		A = matrix;
		do {
			prevA = A;
			QR_Decomposition(A, Q, R);
			S = S * Q;
			A = R * Q;
			count++;

			diff = Vector<double, N>(A.diagonal().asDouble() - prevA.diagonal().asDouble()).length();
		} while (count < 50 && abs(diff) > tolerance);

		if (count == 50) {
			qCritical() << "Warning, upper triangular did not converge to a solution";
		}

		triangularMatrix = std::move(A);
		SMatrix = std::move(S);
	}
	//-----------------------------------------------------------------------------------------------------------------
    static void eigenDecomposition(const SquareMatrix<D, N>& matrix,
        Vector<D, N>& eigenValues,
        SquareMatrix<D, N>& eigenVectors,
        double tolerance) {
#ifdef LINALG_USE_EIGEN  
        Q_UNUSED(tolerance);
        Eigen::Map<const Eigen::Matrix<D, N, N>> map(matrix.getData());

        if (map.isApprox(map.transpose())) {
            // If matrix is self-adjoint
            Eigen::SelfAdjointEigenSolver<Eigen::Matrix<D, N, N>> es;
            es.compute(map);
            Eigen::Matrix<D, N, 1> vec = es.eigenvalues().real();
            Eigen::Matrix<D, N, N> mtx = es.eigenvectors().real();

            eigenValues = Vector<D, N>(vec.data());
            eigenVectors = SquareMatrix<D, N>(mtx.data());

            if (eigenValues.hasNegative()) {
                qCritical() << "Warning, matrix is not positive-definite, eigenvectors may be inaccurate.";
            }
        }
        else {
            // General eigenvalue solver
            Eigen::EigenSolver<Eigen::Matrix<D, N, N>> es;
            es.compute(map, true);
            Eigen::Matrix<D, N, 1> vec = es.eigenvalues().real();
            Eigen::Matrix<D, N, N> mtx = es.eigenvectors().real();

            eigenValues = Vector<D, N>(vec.data());
            eigenVectors = SquareMatrix<D, N>(mtx.data());
        }

#else
		// Get upper triangular form of matrix
		SquareMatrix A, S;
		SquareMatrix<D, N>::toUpperTriangular(matrix, A, S, tolerance);

		// Get eigenvalues //////////////////////////////////////////////////////////////////////////-
		eigenValues = A.diagonal();

		// Get eigenvectors //////////////////////////////////////////////////////////////////////////
		if (eigenValues.hasNegative()) {
			qCritical() << "Warning, matrix is not positive-definite, eigenvectors may be inaccurate.";
		}

		eigenVectors = S;

#endif
	}

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Function to get cofactor matrix of given entry
    /// @note See: https://www.geeksforgeeks.org/adjoint-inverse-matrix/
    SquareMatrix<D, N> getCofactor(int rowNum, int colNum, int dim) const{
        SquareMatrix<D, N> cofactor;
        int i = 0, j = 0;

        // Looping for each element of the matrix 
        for (int row = 0; row < dim; row++)
        {
            for (int col = 0; col < dim; col++)
            {
                //  Copying into temporary matrix only those element 
                //  which are not in given row and column 
                if (row != rowNum && col != colNum)
                {
                    cofactor(i, j++) = (*this)(row, col);

                    // Row is filled, so increase row index and 
                    // reset col index 
                    if (j == dim - 1){
                        j = 0;
                        i++;
                    }
                }
            }
        }

        return cofactor;
    }

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Recursive function for finding determinant of the matrix.
    /// [in] n is current dimension of the matrix
    /// @note See: https://www.geeksforgeeks.org/adjoint-inverse-matrix/
    inline D determinant(int n = N) const{
        D det = 0; // Initialize result 

        //  Base case : if matrix contains single element 
        if (n == 1) return (*this)(0, 0);

        // To store sign multiplier and cofactors 
        int sign = 1;
        SquareMatrix<D, N> temp;

         // Iterate for each element of first row 
        for (int f = 0; f < n; f++){
            // Getting Cofactor of A[0][f] 
            temp = getCofactor(0, f, n);
            det += sign * (*this)(0, f) * temp.determinant(n - 1);

            // terms are to be added with alternate sign 
            sign = -sign;
        }

        return det;
    }

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Function to get adjoint of the matrix
    /// @note See: https://www.geeksforgeeks.org/adjoint-inverse-matrix/
    inline SquareMatrix<D, N> adjoint() const{
        SquareMatrix<D, N> adj;

        // Adjoint is one for the 1D case
        if (N == 1){
            adj(0, 0) = 1;
            return adj;
        }

        // temp is used to store cofactors of the matrix
        int sign = 1;
        SquareMatrix<D, N> temp;

        for (int i = 0; i < N; i++){
            for (int j = 0; j < N; j++){
                // Get cofactor of A[i][j] 
                temp = getCofactor(i, j, N);

                // sign of adj[j][i] positive if sum of row 
                // and column indexes is even. 
                sign = ((i + j) % 2 == 0) ? 1 : -1;

                // Interchanging rows and columns to get the 
                // transpose of the cofactor matrix 
                adj(j, i) = sign * temp.determinant(N - 1);
            }
        }

        return adj;
    }

    //-----------------------------------------------------------------------------------------------------------------
    /// @brief Function to calculate inverse of the matrix
    /// @note See: https://www.geeksforgeeks.org/adjoint-inverse-matrix/
    inline SquareMatrix<D, N> inversed() const {
#ifdef LINALG_USE_EIGEN
        SquareMatrix<D, N> inverse = SquareMatrix<D, N>::EmptyMatrix();
        inverse.map() = getMap().inverse();
        return inverse;
#else
        // Find determinant
        D det = determinant();
        if (det == 0){
            throw("Singular matrix, can't find its inverse");
        }

        // Find adjoint 
        SquareMatrix<D, N> adj = adjoint();

        // Find Inverse using formula "inverse(A) = adj(A)/det(A)" 
        SquareMatrix<D, N> inverse = adj / det;

        return inverse;
#endif
    }
    //-----------------------------------------------------------------------------------------------------------------
/// @brief Function to calculate inverse of the matrix
/// @note See: https://www.geeksforgeeks.org/adjoint-inverse-matrix/
    inline void inversed(SquareMatrix<D, N>& out) const {
#ifdef LINALG_USE_EIGEN
        out.map() = getMap().inverse();
#else
        // Find determinant
        D det = determinant();
        if (det == 0) {
            throw("Singular matrix, can't find its inverse");
        }

        // Find adjoint and find Inverse using formula "inverse(A) = adj(A)/det(A)" 
        out = adj / det;
#endif
    }

    /// @}

protected:

    explicit SquareMatrix(bool fill) : Matrix() {
        if (fill) {
            setToIdentity();
        }
    }

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{  

    friend class ShaderProgram;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs based on generic matrix
typedef SquareMatrix<float, 2> Matrix2x2;
typedef SquareMatrix<float, 3> Matrix3x3;
typedef SquareMatrix<float, 4> Matrix4x4;
typedef SquareMatrix<double, 2> Matrix2x2d;
typedef SquareMatrix<double, 3> Matrix3x3d;
typedef SquareMatrix<double, 4> Matrix4x4d;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Convert standard vector of matrices to and from JSON
QJsonValue matrixVecAsJson(const std::vector<Matrix4x4>& vec);
std::vector<Matrix4x4> matrixVecFromJson(const QJsonValue& json);




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespacing

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Declare metatypes
Q_DECLARE_METATYPE(rev::Matrix2x2)
Q_DECLARE_METATYPE(rev::Matrix3x3)
Q_DECLARE_METATYPE(rev::Matrix4x4)
Q_DECLARE_METATYPE(rev::Matrix2x2d)
Q_DECLARE_METATYPE(rev::Matrix3x3d)
Q_DECLARE_METATYPE(rev::Matrix4x4d)

#endif
