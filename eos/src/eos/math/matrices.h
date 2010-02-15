#ifndef EOS_MATH_MATRICES_H
#define EOS_MATH_MATRICES_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file matrices.h
/// Provides the matrix clesses, both templated to a fixed size and variable size,
/// all templated by numerical type. Operations are provided independently of both 
/// types, so they only have to be coded once.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/math/functions.h"
#include "eos/io/inout.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// A Matrix class with templated dimensions and templated type. Provides only 
/// basic operations so all proper operations can be templated to work on both
/// this class and the Matrix classes.
template <nat32 ROWS, nat32 COLS = ROWS, typename T = real32>
class EOS_CLASS Mat
{
 public:
  /// The data item type, so it can be used by templated code.
   typedef T type;

  /// The number of rows for when accessed by templated code.
   static const nat32 rows = ROWS;

  /// The number of rows for when accessed by templated code.
   static const nat32 cols = COLS;


  /// Does not initialise anything, leaves all values random.
   Mat() {}
   
  /// &nbsp;
   Mat(T var[ROWS][COLS]) 
   {
    for (nat32 r=0;r<ROWS;r++)
    {
     for (nat32 c=0;c<COLS;c++) data[r][c] = var[r][c];	    
    }
   }
   
  /// &nbsp;
   Mat(const Mat<ROWS,COLS,T> & rhs)
   {
    for (nat32 r=0;r<ROWS;r++)
    {
     for (nat32 c=0;c<COLS;c++) data[r][c] = rhs.data[r][c];	    
    }
   }
   
  /// &nbsp;
   ~Mat() {}
   
   
  /// &nbsp;
   Mat<ROWS,COLS,T> & operator = (const Mat<ROWS,COLS,T> & rhs)
   {
    for (nat32 r=0;r<ROWS;r++)
    {
     for (nat32 c=0;c<COLS;c++) data[r][c] = rhs.data[r][c];	    
    }
    return *this;
   }


  /// &nbsp;
   nat32 Rows() const {return ROWS;}
   
  /// &nbsp;
   nat32 Cols() const {return COLS;}
   
  /// &nbsp;
   T * operator [] (nat32 row) {return data[row];}
   
  /// &nbsp;
   const T * operator [] (nat32 row) const {return data[row];}


  /// &nbsp;
   Mat<ROWS,COLS,T> & operator *= (T rhs)
   {
    for (nat32 r=0;r<ROWS;r++)
    {
     for (nat32 c=0;c<COLS;c++) data[r][c] *= rhs;
    }
    return *this;
   }

  /// &nbsp;
   Mat<ROWS,COLS,T> & operator /= (T rhs)
   {
    for (nat32 r=0;r<ROWS;r++)
    {
     for (nat32 c=0;c<COLS;c++) data[r][c] /= rhs;
    }
    return *this;
   }

  /// &nbsp;
   Mat<ROWS,COLS,T> & operator += (const Mat<ROWS,COLS,T> & rhs)
   {
    for (nat32 r=0;r<ROWS;r++)
    {
     for (nat32 c=0;c<COLS;c++) data[r][c] += rhs.data[r][c];	    
    }
    return *this;
   }

  /// &nbsp;
   Mat<ROWS,COLS,T> & operator -= (const Mat<ROWS,COLS,T> & rhs)
   {
    for (nat32 r=0;r<ROWS;r++)
    {
     for (nat32 c=0;c<COLS;c++) data[r][c] -= rhs.data[r][c];	    
    }
    return *this;
   }
   
   
  /// Swaps the 2 rows with given indexes, accheives nothing if the two indexes are identical.
   void SwapRows(nat32 a,nat32 b)
   {
    T temp; 
    for (nat32 i=0;i<Cols();i++) 
    {temp = (*this)[a][i]; (*this)[a][i] = (*this)[b][i]; (*this)[b][i] = temp;}
   }
   
  /// Swaps the 2 columns with given indexes, accheives nothing if the two indexes are identical.
   void SwapCols(nat32 a,nat32 b)
   {
    T temp; 
    for (nat32 i=0;i<Rows();i++) 
    {temp = (*this)[i][a]; (*this)[i][a] = (*this)[i][b]; (*this)[i][b] = temp;}
   }
   
  /// Copys a row from another matrix into a specified row.
   template <nat32 ORC>
   void GetRow(nat32 rowTo,const Mat<ORC,COLS,T> & rhs,nat32 rowFrom)
   {
    for (nat32 i=0;i<COLS;i++) (*this)[rowTo][i] = rhs[rowFrom][i];
   }

  /// Copys a column from another matrix into a specified column.
   template <nat32 OCC>
   void GetCol(nat32 colTo,const Mat<ROWS,OCC,T> & rhs,nat32 colFrom)
   {
    for (nat32 i=0;i<ROWS;i++) (*this)[i][colTo] = rhs[i][colFrom];
   }


  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::math::Mat<" << ROWS << "," << COLS << "," << typestring<T>() << ">");
    return ret;
   }


 private:
  T data[ROWS][COLS];
};

//------------------------------------------------------------------------------
/// A Matrix class templated on type only, has variable size. Provides only
/// minimal functionality as all the real functionality is provided independent
/// of Matrix type, be it this or Mat.
template <typename T = real32>
class EOS_CLASS Matrix
{
 public:
  /// The data item type, so it can be used by templated code.
   typedef T type;


  /// Constructs the matrix as size 0, call SetSize before use.
   Matrix():rows(0),cols(0),data(null<T*>()) {}

  /// Constructs the matrix leaving the data as random values.
  /// \param r Rows of matrix.
  /// \param c Columns of matrix.
   Matrix(nat32 r,nat32 c):rows(r),cols(c),data(new T[r*c]) {}
   
  /// &nbsp;
   template <nat32 ROWS, nat32 COLS>
   Matrix(const Mat<ROWS,COLS,T> & rhs)
   :rows(ROWS),cols(COLS),data(new T[ROWS*COLS])
   {
    for (nat32 r=0;r<rows;r++)
    {
     for (nat32 c=0;c<cols;c++) (*this)[r][c] = rhs[r][c];	    
    }	   
   }
   
  /// &nbsp;
   Matrix(const Matrix<T> & rhs)
   :rows(rhs.rows),cols(rhs.cols),data(new T[rhs.rows*rhs.cols])
   {
    for (nat32 r=0;r<rows;r++)
    {
     for (nat32 c=0;c<cols;c++) (*this)[r][c] = rhs[r][c];
    }	   
   }   
   
  /// &nbsp;
   ~Matrix() {delete[] data;}


  /// &nbsp;
   template <nat32 ROWS, nat32 COLS>
   Matrix<T> & operator = (const Mat<ROWS,COLS,T> & rhs)
   {
    SetSize(rhs.Rows(),rhs.Cols());
    for (nat32 r=0;r<rows;r++)
    {
     for (nat32 c=0;c<cols;c++) (*this)[r][c] = rhs[r][c];
    }
    return *this;	   
   }

  /// &nbsp;
   Matrix<T> & operator = (const Matrix<T> & rhs)
   {
    SetSize(rhs.Rows(),rhs.Cols());
    for (nat32 r=0;r<rows;r++)
    {
     for (nat32 c=0;c<cols;c++) (*this)[r][c] = rhs[r][c];
    }
    return *this;	   
   }   


  /// &nbsp;
   nat32 Rows() const {return rows;}
   
  /// &nbsp;
   nat32 Cols() const {return cols;}

  /// &nbsp;
   void SetSize(nat32 r,nat32 c)
   {
    if ((r!=rows)||(c!=cols))
    {
     rows = r; cols = c;
     delete[] data;
     data = new T[rows*cols];	    
    }
   }

  /// &nbsp;
   T * operator [] (nat32 row) {return &data[row*cols];}

  /// &nbsp;
   const T * operator [] (nat32 row) const {return &data[row*cols];}


  /// Swaps the 2 rows with given indexes, accheives nothing if the two indexes are identical.
   void SwapRows(nat32 a,nat32 b)
   {
    log::Assert((a<rows)&&(b<rows));
    T temp; 
    for (nat32 i=0;i<Cols();i++)
    {temp = (*this)[a][i]; (*this)[a][i] = (*this)[b][i]; (*this)[b][i] = temp;}
   }
   
  /// Swaps the 2 columns with given indexes, accheives nothing if the two indexes are identical.
   void SwapCols(nat32 a,nat32 b)
   {
    log::Assert((a<cols)&&(b<cols));   
    T temp; 
    for (nat32 i=0;i<Rows();i++) 
    {temp = (*this)[i][a]; (*this)[i][a] = (*this)[i][b]; (*this)[i][b] = temp;}
   }
   
 
  /// &nbsp;
   Matrix<T> & operator *= (T rhs)
   {
    for (nat32 r=0;r<rows;r++)
    {
     for (nat32 c=0;c<cols;c++) (*this)[r][c] += rhs;
    }
    return *this;	   
   }

  /// &nbsp;
   Matrix<T> & operator += (const Matrix<T> & rhs)
   {
    log::Assert((rows==rhs.rows)&&(cols==rhs.cols));
    for (nat32 r=0;r<rows;r++)
    {
     for (nat32 c=0;c<cols;c++) (*this)[r][c] += rhs[r][c];
    }
    return *this;	   
   }
   
  /// &nbsp;
   Matrix<T> & operator -= (const Matrix<T> & rhs)
   {
    log::Assert((rows==rhs.rows)&&(cols==rhs.cols));   
    for (nat32 r=0;r<rows;r++)
    {
     for (nat32 c=0;c<cols;c++) (*this)[r][c] -= rhs[r][c];
    }
    return *this;	   
   }   


  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::math::Matrix<" << typestring<T>() << ">");
    return ret;
   }


 private:	
  nat32 rows;
  nat32 cols;
  T * data;
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// IO stream integration for the above classes...
namespace eos
{
 namespace io
 {

  template <typename T, nat32 VR, nat32 VC, typename VT>
  inline T & StreamRead(T & lhs,math::Mat<VR,VC,VT> & rhs,Binary)
  {   
   for (nat32 r=0;r<rhs.Rows();r++)
   {
    for (nat32 c=0;c<rhs.Cols();c++)
    {
     lhs >> rhs[r][c];
    }
   }
   return lhs;
  }
  
  template <typename T, nat32 VR, nat32 VC, typename VT>
  inline T & StreamRead(T & lhs,math::Mat<VR,VC,VT> & rhs,Text)
  {
   SkipWS(lhs);
   cstrchar p;
   if ((lhs.Peek(&p,1)!=1)||(p!='[')) {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   for (nat32 r=0;r<rhs.Rows();r++)
   {
    for (nat32 c=0;c<rhs.Cols();c++)
    {
     lhs >> rhs[r][c];
     if (c+1!=rhs.Cols())
     {
      if ((lhs.Peek(&p,1)!=1)||(p!=',')) {lhs.SetError(true); return lhs;}	  
      lhs.Skip(1);
     }
    }
    if (r+1!=rhs.Rows())
    {
     if ((lhs.Peek(&p,1)!=1)||(p!=';')) {lhs.SetError(true); return lhs;}	  
     lhs.Skip(1);   
    }
   }
   if ((lhs.Peek(&p,1)!=1)||(p!=']')) {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   return lhs;
  }
  
  template <typename T, nat32 VR, nat32 VC, typename VT>
  inline T & StreamWrite(T & lhs,const math::Mat<VR,VC,VT> & rhs,Binary)
  {
   for (nat32 r=0;r<rhs.Rows();r++)
   {
    for (nat32 c=0;c<rhs.Cols();c++)
    {
     lhs << rhs[r][c];	 
    }
   }
   return lhs;
  }
  
  template <typename T, nat32 VR, nat32 VC, typename VT>
  inline T & StreamWrite(T & lhs,const math::Mat<VR,VC,VT> & rhs,Text)
  {
   lhs << "[";
   for (nat32 r=0;r<rhs.Rows();r++)
   {
    for (nat32 c=0;c<rhs.Cols();c++)
    {
     lhs << rhs[r][c];
     if (c+1!=rhs.Cols()) lhs << ",";
    }
    if (r+1!=rhs.Rows()) lhs << ";";
   }
   lhs << "]";
   return lhs;
  }

 
  template <typename T, typename VT>
  inline T & StreamRead(T & lhs,math::Matrix<VT> & rhs,Binary)
  {   
   for (nat32 r=0;r<rhs.Rows();r++)
   {
    for (nat32 c=0;c<rhs.Cols();c++)
    {
     lhs >> rhs[r][c];
    }
   }
   return lhs;
  }
  
  template <typename T, typename VT>
  inline T & StreamRead(T & lhs,math::Matrix<VT> & rhs,Text)
  {
   SkipWS(lhs);
   cstrchar p;
   if ((lhs.Peek(&p,1)!=1)||(p!='[')) {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   for (nat32 r=0;r<rhs.Rows();r++)
   {
    for (nat32 c=0;c<rhs.Cols();c++)
    {
     lhs >> rhs[r][c];
     if (c+1!=rhs.Cols())
     {
      if ((lhs.Peek(&p,1)!=1)||(p!=',')) {lhs.SetError(true); return lhs;}	  
      lhs.Skip(1);
     }
    }
    if (r+1!=rhs.Rows())
    {
     if ((lhs.Peek(&p,1)!=1)||(p!=';')) {lhs.SetError(true); return lhs;}	  
     lhs.Skip(1);    
    }
   }
   if ((lhs.Peek(&p,1)!=1)||(p!=']')) {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   return lhs;
  }
  
  template <typename T, typename VT>
  inline T & StreamWrite(T & lhs,const math::Matrix<VT> & rhs,Binary)
  {
   for (nat32 r=0;r<rhs.Rows();r++)
   {
    for (nat32 c=0;c<rhs.Cols();c++)
    {
     lhs << rhs[r][c];	 
    }
   }
   return lhs;
  }
  
  template <typename T, typename VT>
  inline T & StreamWrite(T & lhs,const math::Matrix<VT> & rhs,Text)
  {
   lhs << "[";
   for (nat32 r=0;r<rhs.Rows();r++)
   {
    for (nat32 c=0;c<rhs.Cols();c++)
    {
     lhs << rhs[r][c];
     if (c+1!=rhs.Cols()) lhs << ",";
    }
    if (r+1!=rhs.Rows()) lhs << ";";
   }
   lhs << "]";
   return lhs;
  } 

 };
};

//------------------------------------------------------------------------------
#endif
