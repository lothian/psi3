#ifndef _psi_src_lib_libmints_matrix_h_
#define _psi_src_lib_libmints_matrix_h_

/*!
    \file libmints/matrix.h
    \ingroup MINTS
*/

#include <cstdio>
#include <string>
#include <cstring>

#include <libmints/vector.h>
#include <libmints/ref.h>

#include <libpsio/psio.hpp>

extern FILE *outfile;

namespace psi {

class MatrixFactory;
class SimpleMatrix;

//! Makes using matrices just a little earlier.
//! Using a matrix factory makes creating these a breeze.
class Matrix {
protected:
    /// Matrix data
    double ***matrix_;
    /// Number of irreps
    int nirreps_;
    /// Rows per irrep array
    int *rowspi_;
    /// Columns per irrep array
    int *colspi_;
    /// Name of the matrix
    std::string name_;

    /// Allocates matrix_
    void alloc();
    /// Release matrix_
    void release();

    /// Copies data from the passed matrix to this matrix_
    void copy_from(double ***);

    /// allocate a block matrix -- analogous to libciomr's block_matrix
    static double** matrix(int nrow, int ncol) {
        double** mat = (double**) malloc(sizeof(double*)*nrow);
        const size_t size = sizeof(double)*nrow*ncol;
        mat[0] = (double*) malloc(size);
        //bzero((void*)mat[0],size);
        memset((void *)mat[0], '\0', size);
        for(int r=1; r<nrow; ++r) mat[r] = mat[r-1] + ncol;
        return mat;
    }
    /// free a (block) matrix -- analogous to libciomr's free_block
    static void free(double** Block) {
        ::free(Block[0]);  ::free(Block);
    }

    void print_mat(double **a, int m, int n, FILE *out);

public:
    /// Default constructor, zeros everything out
    Matrix();
    /// Constructor, zeros everything out, sets name_
    Matrix(std::string name);
    /// Explicit copy reference constructor
    explicit Matrix(const Matrix& copy);
    Matrix(Matrix& copy);
    
    /// Explicit copy pointer constructor
    explicit Matrix(const Matrix* copy);
    /// Constructor, sets up the matrix
    Matrix(int nirreps, int *rowspi, int *colspi);
    /// Constructor, sets name_, and sets up the matrix
    Matrix(std::string name, int nirreps, int *rowspi, int *colspi);

    /// Destructor, frees memory
    ~Matrix();

    /// Initializes a matrix
    void init(int nirreps, int *rowspi, int *colspi, std::string name = "");
    
    /// Creates an exact copy of the matrix and returns it.
    Matrix* clone() const;
    /// Copies cp's data onto this
    void copy(Matrix* cp);
    void copy(Matrix& cp);
    void copy(const Matrix& cp);
    void copy(const Matrix* cp);

    /// Load a matrix from a PSIO object from fileno with tocentry of size nso
    bool load(psi::PSIO* psio, unsigned int fileno, char *tocentry, int nso);

    /// Saves the matrix in ASCII format to filename
    void save(const char *filename, bool append=true, bool saveLowerTriangle = true, bool saveSubBlocks=false);
    void save(std::string filename, bool append=true, bool saveLowerTriangle = true, bool saveSubBlocks=false) {
        save(filename.c_str(), append, saveLowerTriangle, saveSubBlocks);
    }
    /// Saves the block matrix to PSIO object with fileno and with the toc position of the name of the matrix
    void save(psi::PSIO* psio, unsigned int fileno, bool saveSubBlocks=true);

    /// Set every element of matrix_ to val
    void set(double val);
    /// Copies lower triangle tri to matrix_, calls tri_to_sq
    void set(const double *tri);
    /// Copies sq to matrix_
    void set(const double **sq);
    /// Copies sq to matrix_
    void set(SimpleMatrix *sq);
    /// Set a single element of matrix_
    void set(int h, int m, int n, double val) { matrix_[h][m][n] = val; }
    /// Set the diagonal of matrix_ to vec
    void set(Vector *vec);
    void set(Vector& vec);
    /// Returns a single element of matrix_
    double get(int h, int m, int n) { return matrix_[h][m][n]; }
    /// Returns matrix_
    double **to_block_matrix() const;
    /// Converts this to a full non-symmetry-block matrix
    SimpleMatrix *to_simple_matrix();

    /// Sets the name of the matrix, used in print(...) and save(...)
    void set_name(std::string name) {
        name_ = name;
    };

    /// Print the matrix using print_mat
    void print(FILE *out = outfile, char *extra=NULL);
    /// Print the matrix with corresponding eigenvalues below each column
    void eivprint(Vector *values, FILE *out = outfile);
    void eivprint(Vector &values, FILE *out = outfile);
    
    /// Returns the rows per irrep array
    int *rowspi() const {
        return rowspi_;
    }
    /// Returns the columns per irrep array
    int *colspi() const {
        return colspi_;
    }
    /// Returns the number of irreps
    int nirreps() const {
        return nirreps_;
    }
    /// Set this to identity
    void set_to_identity();
    /// Zeros this out
    void zero();
    /// Zeros the diagonal
    void zero_diagonal();

    // Math routines
    /// Returns the trace of this
    double trace();
    /// Creates a new matrix which is the transpose of this
    Matrix *transpose();

    /// Adds a matrix to this
    void add(const Matrix*);
    void add(const Matrix&);
    
    /// Subtracts a matrix from this
    void subtract(const Matrix*);
    /// Multiplies the two arguments and adds their result to this
    void accumulate_product(const Matrix*, const Matrix*);
    /// Scales this matrix
    void scale(double);
    /// Returns the sum of the squares of this
    double sum_of_squares();
    /// Add val to an element of this
    void add(int h, int m, int n, double val) { matrix_[h][m][n] += val; }
    /// Scale row m of irrep h by a
    void scale_row(int h, int m, double a);
    /// Scale column n of irrep h by a
    void scale_column(int h, int n, double a);
    /// Transform a by transformer save result to this
    void transform(Matrix* a, Matrix* transformer);
    /// Transform this by transformer
    void transform(Matrix* transformer);
    /// Back transform a by transformer save result to this
    void back_transform(Matrix* a, Matrix* transformer);
    /// Back transform this by transformer
    void back_transform(Matrix* transformer);

    /// Returns the vector dot product of this by rhs
    double vector_dot(Matrix* rhs);

    /// General matrix multiply, saves result to this
    void gemm(bool transa, bool transb, double alpha, const Matrix* a, const Matrix* b, double beta);
    /// Diagonalize this places eigvectors and eigvalues must be created by caller.
    void diagonalize(Matrix* eigvectors, Vector* eigvalues);
    
    // Reference versions of the above functions:
    
    /// Transform a by transformer save result to this
    void transform(Matrix& a, Matrix& transformer);
    /// Transform this by transformer
    void transform(Matrix& transformer);
    /// Back transform a by transformer save result to this
    void back_transform(Matrix& a, Matrix& transformer);
    /// Back transform this by transformer
    void back_transform(Matrix& transformer);

    /// Returns the vector dot product of this by rhs
    double vector_dot(Matrix& rhs);

    /// General matrix multiply, saves result to this
    void gemm(bool transa, bool transb, double alpha, const Matrix& a, const Matrix& b, double beta);
    /// Diagonalize this places eigvectors and eigvalues must be created by caller.
    void diagonalize(Matrix& eigvectors, Vector& eigvalues);
    
    const Matrix operator*(const Matrix& rhs) const {
        Matrix r(*this);
        r.zero();
        r.accumulate_product(this, &rhs);
        return r;
    }

    const Matrix operator*(const double rhs) const {
        Matrix r(*this);
        r.zero();
        r.scale(rhs);
        return r;
    }
    
    const Matrix operator-(const Matrix& rhs) const {
        Matrix r(*this);
        r.subtract(&rhs);
        return r;
    }

    const Matrix operator+(const Matrix& rhs) const {
        Matrix r(*this);
        r.add(&rhs);
        return r;
    }
    
    Matrix& operator=(const Matrix& rhs) {
        if (this != &rhs) {
            this->copy(rhs);
        }
        
        return *this;
    }
};


//! Simple matrix class. Not symmetry blocked.
class SimpleMatrix
{
protected:
    /// Matrix data
    double **matrix_;
    /// Number of rows and columns
    int rows_, cols_;
    /// Nae of the matrix
    std::string name_;

    /// Allocates matrix_
    void alloc();
    /// Releases matrix_
    void release();

    /// Copies data from the passed matrix to this matrix_
    void copy_from(double **);

    /// allocate a block matrix -- analogous to libciomr's block_matrix
    static double** matrix(int nrow, int ncol) {
        double** mat = (double**) malloc(sizeof(double*)*nrow);
        const size_t size = sizeof(double)*nrow*ncol;
        mat[0] = (double*) malloc(size);
        //bzero((void*)mat[0],size);
        memset((void *)mat[0], '\0', size);
        for(int r=1; r<nrow; ++r) mat[r] = mat[r-1] + ncol;
        return mat;
    }
    /// free a (block) matrix -- analogous to libciomr's free_block
    static void free(double** Block) {
        ::free(Block[0]);  ::free(Block);
    }

public:
    /// Default constructor, zeros everything out
    SimpleMatrix();
    /// Constructor, zeros everything out, sets name_
    SimpleMatrix(std::string name);
    /// Explicit copy reference constructor
    explicit SimpleMatrix(const SimpleMatrix& copy);
    /// Explicit copy pointer constructor
    explicit SimpleMatrix(const SimpleMatrix* copy);
    /// Constructor, sets up the matrix
    SimpleMatrix(int rows, int cols);
    /// Constructor, sets name_, and sets up the matrix
    SimpleMatrix(std::string name, int rows, int cols);
    /// Converts Matrix reference to SimpleMatrix
    SimpleMatrix(const Matrix& copy);
    /// Converts Matrix pointer to SimpleMatrix
    SimpleMatrix(const Matrix* copy);

    /// Destructor, frees memory
    ~SimpleMatrix();

    /// Initializes a matrix
    void init(int rowspi, int colspi, std::string name = "");

    /// Creates an exact copy of the matrix and returns it.
    SimpleMatrix* clone() const;
    /// Copies cp's data onto this
    void copy(SimpleMatrix* cp);

    /// Set every element of this to val
    void set(double val);
    /// Copies lower triangle tri to matrix_
    void set(const double *tri);
    /// Set a single element of matrix_
    void set(int m, int n, double val) { matrix_[m][n] = val; }
    void set(SimpleVector *vec);
    void set(double **mat);

    /// Sets the diagonal of matrix_ to vec
    double get(int m, int n) { return matrix_[m][n]; }
    /// Returns matrix_
    double **to_block_matrix() const;
    /// Sets the name of the matrix
    void set_name(std::string name) { name_ = name; }

    /// Prints the matrix with print_mat
    void print(FILE *out = outfile);
    /// Print the matrix with corresponding eigenvalues below each column
    void eivprint(SimpleVector *values, FILE *out = outfile);
    /// The number of rows
    int rows() const { return rows_; }
    /// The number of columns
    int cols() const { return cols_; }
    /// Set matrix to identity
    void set_to_identity();
    /// Zero out the matrix
    void zero();
    /// Zero out the diagonal
    void zero_diagonal();

    /// Returns the trace of this
    double trace() const;
    /// Create a new SimpleMatrix which is the transpose of this
    SimpleMatrix *transpose();

    /// Add a matrix to this
    void add(const SimpleMatrix*);
    /// Subtracts a matrix from this
    void subtract(const SimpleMatrix*);
    /// Multiples the two arguments and adds their result to this
    void accumulate_product(const SimpleMatrix*, const SimpleMatrix*);
    /// Scales this matrix
    void scale(double);
    /// Returns the sum of the squares of this
    double sum_of_squares();
    /// Add val to an element of this
    void add(int m, int n, double val) { matrix_[m][n] += val; }
    /// Scale row m by a
    void scale_row(int m, double a);
    /// Scale column n by a
    void scale_column(int n, double a);
    /// Transform a by transformer save result to this
    void transform(SimpleMatrix* a, SimpleMatrix* transformer);
    /// Transform this by transformer
    void transform(SimpleMatrix* transformer);
    /// Back transform a by transformer save result to this
    void back_transform(SimpleMatrix* a, SimpleMatrix* transformer);
    /// Back transform this by transformer
    void back_transform(SimpleMatrix* transformer);

    /// Return the vector dot product of rhs by this
    double vector_dot(SimpleMatrix* rhs);

    /// General matrix multiply, saves result to this
    void gemm(bool transa, bool transb, double alpha, const SimpleMatrix* a, const SimpleMatrix* b, double beta);
    /// Diagonalize this, eigvector and eigvalues must be created by caller.
    void diagonalize(SimpleMatrix* eigvectors, SimpleVector* eigvalues);

    /// Saves the block matrix to PSIO object with fileno and with the toc position of the name of the matrix
    void save(psi::PSIO* psio, unsigned int fileno);
    void save(psi::PSIO& psio, unsigned int fileno);
    
    /// Saves the matrix in ASCII format to filename
    void save(const char *filename, bool append=true, bool saveLowerTriangle = true);
    /// Saves the matrix in ASCII format to filename
    void save(std::string filename, bool append=true, bool saveLowerTriangle = true);
    
    friend class Matrix;
};

#include "matrix_i.cc"

}

#endif // MATRIX_H
