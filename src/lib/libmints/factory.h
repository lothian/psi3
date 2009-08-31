#ifndef _psi_src_lib_libmints_factory_h_
#define _psi_src_lib_libmints_factory_h_

/*!
    \file libmints/factory.h
    \ingroup MINTS
*/

#include <libmints/vector.h>
#include <libmints/matrix.h>

#include <libchkpt/chkpt.hpp>
#include <libpsio/psio.hpp>

namespace psi {

/// A class for creating Matrix, SimpleMatrix, Vector, and SimpleVector objects.
/// The objects this factory creates can automatically be sized based on information
/// from checkpoint.    
class MatrixFactory {
    /// Number of irreps
    int nirreps_;
    /// Number of orbitals
    int nso_;
    /// Number of rows per irrep
    int *rowspi_;
    /// Number of columns per irrep
    int *colspi_;
    
public:
    /// Default constructor, does nothing.
    MatrixFactory();
    /// Copy constructor.
    MatrixFactory(const MatrixFactory& copy);
    ~MatrixFactory();
    
    /// Initializes the matrix factory by creating a chkpt object with a psio reference.
    bool init_with_chkpt(psi::PSIO* psio);
    bool init_with_chkpt(psi::PSIO& psio);
    
    /// Initializes the matrix factory using the given chkpt object.
    bool init_with_chkpt(psi::Chkpt* chkpt);
    bool init_with_chkpt(psi::Chkpt& chkpt);
    
    /// Manually initialize the matrix factory
    bool init_with(int nirreps, int *rowspi, int *colspi);
    
    /// Returns number of irreps
    int nirreps() const {
        return nirreps_;
    }
    
    /// Returns the rows per irrep array
    int *rowspi() const {
        return rowspi_;
    }
    
    /// Returns the number of rows in irrep h
    int nrows(int h) const {
        return rowspi_[h];
    }
    
    /// Returns the columns per irrep array
    int *colspi() const {
        return colspi_;
    }
    
    /// Returns the number of columns in irrep h
    int ncols(int h) const {
        return colspi_[h];
    }
    
    /// Returns the number of orbitals
    int nso() const {
        return nso_;
    }
    
    /// Returns a new Matrix object with default dimensions
    Matrix * create_matrix()
    {
        return new Matrix(nirreps_, rowspi_, colspi_);
    }
    
    void create_matrix(Matrix& mat)
    {
        mat.init(nirreps_, rowspi_, colspi_);
    }
    
    /// Returns a new Matrix object named name with default dimensions
    Matrix * create_matrix(std::string name)
    {
        return new Matrix(name, nirreps_, rowspi_, colspi_);
    }
    
    void create_matrix(Matrix& mat, std::string name)
    {
        mat.init(nirreps_, rowspi_, colspi_, name);
    }
    
    /// Returns a new Vector object with default dimensions
    Vector * create_vector()
    {
        return new Vector(nirreps_, rowspi_);
    }
    
    void create_vector(Vector& vec)
    {
        vec.init(nirreps_, rowspi_);
    }
    
    /// Returns a new SimpleMatrix object with default dimensions
    SimpleMatrix * create_simple_matrix()
    {
        return new SimpleMatrix(nso_, nso_);
    }
    void create_simple_matrix(SimpleMatrix& mat)
    {
        mat.init(nso_, nso_);
    }
    
    /// Returns a new SimpleMatrix object named name with default dimensions
    SimpleMatrix * create_simple_matrix(std::string name)
    {
        return new SimpleMatrix(name, nso_, nso_);
    }
    void create_simple_matrix(SimpleMatrix& mat, std::string name)
    {
        mat.init(nso_, nso_, name);
    }
    
    /// Returns a new SimpleMatrix object named name of size m x n
    SimpleMatrix * create_simple_matrix(std::string name, int m, int n)
    {
        return new SimpleMatrix(name, m, n);
    }
    void create_simple_matrix(SimpleMatrix& mat, std::string name, int m, int n)
    {
        mat.init(m, n, name);
    } 
    
    /// Returns a new SimpleMatrix object with size m x n
    SimpleMatrix * create_simple_matrix(int m, int n)
    {
        return new SimpleMatrix(m, n);
    }
    void create_simple_matrix(SimpleMatrix& mat, int m, int n)
    {
        mat.init(m, n);
    }
    
    /// Returns a new SimpleVector object with default dimension
    SimpleVector * create_simple_vector()
    {
        return new SimpleVector(nso_);
    }
    
    /// Returns a new SimpleVector object with size m
    SimpleVector * create_simple_vector(int m)
    {
        return new SimpleVector(m);
    }
};

}

#endif
