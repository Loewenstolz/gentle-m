/** \file
    \brief Contains the GenBank class and its TGenBankFeature helper class
*/
#include "GenBank.h"
#include <wx/textfile.h>

#include "main.h"

#define TAG_COMPLEMENT 1

/// \brief The item type names
const char * const gb_item_type[VIT_TYPES] =
    {
    "",
    "gene",
    "CDS",
    "rep_origin",
    "promoter",
    "terminator",
    "misc_feature",
    "prot_bind",
    "oriT"
    } ;

/// \brief Constructor
TGenBank::TGenBank ()
    {
    int a ;
    for ( a = 0 ; a < 256 ; a++ ) validseq[a] = isValidSequence ( a ) ;
    for ( a = 0 ; a < 256 ; a++ ) isblank[a] = false ;
    for ( a = 0 ; a < 16 ; a++ ) isblank[a] = true ;
    isblank[' '] = true ;
    for ( a = 0 ; a < 256 ; a++ ) isblankorquote[a] = isblank[a] ;
    isblankorquote['"'] = true ;
    success = false ;
    }

/** \brief Loads text from file, calles parser
    \param s Filename
*/
void TGenBank::load ( const wxString& s )
    {
    vs.Clear() ; // Clearing old data

    // Loading file to memory
    wxFile file ( s , wxFile::read ) ;
    long l = file.Length() ;
    char *t = new char [l+15] ;
    file.Read ( t , l ) ;
    file.Close() ;
    t[l] = '\n' ;
    t[l+1] = 0 ;
    mylog ( "-GenBank import" , "file opened" ) ;

    char *c , *d ;
    vs.Alloc ( l / 60 ) ;
    for ( c = d = t ; *c ; c++ )
        {
        if ( *c == '\n' || *c == '\r' )
            {
            *c = 0 ;
            if ( *d ) vs.Add ( wxString ( d , *wxConvCurrent ) ) ;
            d = c+1 ;
            }
        }
    delete [] t ;
    mylog ( "-GenBank import" , "file added" ) ;

    parseLines () ; // Calling parser
    mylog ( "-GenBank import" , "file parsed" ) ;
    }

/** \brief Calls parser for text
    \param s The text in GenBank format
*/
void TGenBank::paste ( const wxString& s )
    {
    explode ( "\n" , s , vs ) ; // Separate into an array of lines
    parseLines () ; // Calling parser
    }

/// \brief The parser
void TGenBank::parseLines ()
    {
    success = false ;
    vi.Clear () ;
    vi_l.clear () ;
    vs_l.clear () ;

    if ( vs.IsEmpty () || vs[0].Left ( 5 ) != _T("LOCUS") )
        return ;

    int a ;
    vi.Alloc ( vs.GetCount() ) ;
    for ( a = 0 ; a < vs.GetCount() ; a++ )
        vi.Add ( count_blanks ( vs[a] ) ) ;

    // Pre-processing
    int last = -1 ;
    wxArrayString vs2 ;
    wxArrayInt vi2 ;
    vs2.Alloc ( vs.GetCount() ) ;
    vi2.Alloc ( vi.GetCount() ) ;
    for ( a = 0 ; a < vs.GetCount() ; a++ )
        {
        if ( vi[a] == 12 || ( vi[a] == 21 && vs[a].GetChar(21) != '/' ) )
            {
            vs2[last] += " " + trim ( vs[a] ) ;
            }
        else
            {
            last++ ;
            vs2.Add ( trim ( vs[a] ) ) ;
            vi2.Add ( vi[a] ) ;
            }
        }


    // Checking if only one sequence, can be handeled faster
    if ( vs2.GetCount()-1 == vs2.Index ( "//" ) )
        {
        success = true ;
        vs2.RemoveAt ( vs2.GetCount() - 1 ) ;
        vi2.RemoveAt ( vi2.GetCount() - 1 ) ;
        vs_l.push_back ( vs2 ) ;
        vi_l.push_back ( vi2 ) ;
        vs.Clear () ;
        vi.Clear () ;
        vs2.Clear () ;
        vi2.Clear () ;
        return ;
        }

    vs = vs2 ;
    vi = vi2 ;
    vs2.Clear() ;
    vi2.Clear() ;

    wxArrayString t ;
    wxArrayInt ti ;
    t.Alloc ( vs.GetCount() ) ;
    ti.Alloc ( vs.GetCount() ) ;
    for ( a = 0 ; a < vs.GetCount() ; a++ )
        {
        if ( vs[a].GetChar(vi[a]) == '/' && vs[a].GetChar(vi[a]+1) == '/' )
            {
            vs_l.push_back ( t ) ;
            vi_l.push_back ( ti ) ;
            t.Clear () ;
            ti.Clear () ;
            }
        else if ( !trim(vs[a]).IsEmpty() )
            {
            t.Add ( trim ( vs[a] ) ) ;
            ti.Add ( vi[a] ) ;
            }
        }

    if ( t.GetCount() > 0 )
        {
        vs_l.push_back ( t ) ;
        vi_l.push_back ( ti ) ;
        }

    if ( vs_l.size() > 0 )
        {
        success = true ;
        vs = vs_l[0] ;
        vi = vi_l[0] ;
        }
    }

/// \brief Wrapper to map the current sequence to a TVector
void TGenBank::remap ( TVector *v )
    {
    remap ( v , vs , vi ) ;
    }

/** \brief Remaps the given data to a TVector
    \param v Pointer to the TVector
    \param vs The lines
    \param vi The line indentations
*/
void TGenBank::remap ( TVector *v , const wxArrayString &vs , const wxArrayInt &vi )
    {
    int line ;
    wxString k1 ;
    vector <wxArrayString> items ;
    wxString ns ;

    int a = 0 ;

    for ( line = 0 ; line < vs.GetCount() ; line++ ) a += (vi[line]==5) ? 1 : 0 ;
    items.reserve ( a*2 ) ;

    wxString l , l2 ;
    for ( line = 0 ; line < vs.GetCount() ; line++ )
        {
        l = vs[line] ;
        int i = vi[line] ;
        if ( i == 0 ) // New main level keyword
            {
            l += " " ;
            k1 = l.BeforeFirst ( ' ' ) . Upper() ;
            l2 = trim ( l.AfterFirst ( ' ' ) ) ;
            i += l.Length() - l2.Length() ;
            l = l2 ;
            }
        if ( k1 == _T("LOCUS") )
            {
            v->setName ( l.BeforeFirst ( ' ' ) ) ;
            v->setDescription ( l.AfterFirst ( ' ' ) ) ;
            l += " " ; // For substring search
            l.MakeUpper() ;
            if ( l.Contains ( " AA " ) ) v->setType ( TYPE_AMINO_ACIDS ) ;
            else v->setType ( TYPE_VECTOR ) ;
            if ( l.Contains ( _T(" CIRCULAR") ) ) v->setCircular ( true ) ;
            else v->setCircular ( false ) ;
            }
        else if ( k1 == _T("FEATURES") )
            {
            if ( i == 5 )
                {
                items.push_back ( wxArrayString() ) ;
                items[items.size()-1].Add ( l.BeforeFirst ( ' ' ) ) ;
                l2 = l.AfterFirst ( ' ' ) ;
                i += l.Length() - l2.Length() ;
                l = l2 ;
                }
            if ( items.size() ) items[items.size()-1].Add ( l ) ;
            }
        else if ( k1 == _T("ORIGIN") )
            {
            if ( ns.IsEmpty() ) ns.Alloc ( ( vs.GetCount() - line ) * 60 ) ;
            for ( a = 0 ; a < l.length() && !validseq[l.GetChar(a)] ; a++ ) ;
            if ( a < l.length() )
                {
                l2 = l.Mid ( a ) ;
                l2.Replace ( " " , "" ) ;
                ns += l2 ;
                }
            }
        else
            {
            v->addDescription ( l + "\n" ) ;
            }
        }

    v->setSequence ( ns.MakeUpper() ) ;
    v->items.reserve ( items.size() ) ;
    for ( a = 0 ; a < items.size() ; a++ ) addItem ( v , items[a] ) ;
    }

/** \brief Checks if the given char is "-", A-Z, or a-z
    \param a The char to check
*/
bool TGenBank::isValidSequence ( char a )
    {
    if ( a == '-' ) return true ;
    if ( a >= 'A' && a <= 'Z' ) return true ;
    if ( a >= 'a' && a <= 'z' ) return true ;
    return false ;
    }

/** \brief Adds an item to a TVector
    \param v Pointer to the TVector
    \param va Array of lines containing the item, GenBank-encoded
*/
void TGenBank::addItem ( TVector *v , wxArrayString &va )
    {
    TVectorItem i ( "" , "" , 1 , 1 , VIT_MISC ) ; // Dummy values
    int a ;

    // Type
    if ( va[0].MakeLower() == _T("source") ) return ;
/*    for ( a = 0 ; a < VIT_TYPES ; a++ )
        {
        if ( va[0].CmpNoCase ( gb_item_type[a] ) == 0 )
           {
           i.setType ( a ) ;
           break ;
           }
        }
    if ( a == VIT_TYPES )*/ i.setType ( va[0] ) ;

    // Properties
    i.desc.Alloc ( 1000 ) ;
    for ( a = 1 ; a < va.GetCount() ; a++ )
        {
        if ( va[a].GetChar(0) != '/' ) continue ;
        wxString p = va[a].Mid ( 1 , va[a].First ( '=' ) - 1 ) ;
        if ( p.IsEmpty() ) continue ;
        multitrim ( p , true ) ;
        p.MakeLower() ;
        wxString v = va[a].AfterFirst ( '=' ) ;
        multitrim ( v , true ) ;
        if ( p == _T("name") || p == _T("standard_name") || p == _T("gene") || p == _T("protein_id") || p == _T("region_name") )
           i.name = v ;
        else
           {
           if ( p == _T("product") || p == _T("organism") || p == _T("db_xref") || p == _T("note") ||
                p == _T("mol_type") || p == _T("chromosome") || p == _T("bound_moiety") )
              {
              if ( !i.desc.IsEmpty() ) i.desc += "\n" ;
              if ( p == _T("chromosome") || p == _T("db_xref") )
                 {
                 i.desc += p ;
                 i.desc += _T(" : ") ;
                 }
              wxString v2 = v ;
              if ( p == _T("db_xref") )
                 {
                 v2 = v.BeforeFirst ( ':' ) . Upper () ;
                 v2.Trim () ;
                 if ( v2 == _T("GI") )
                    {
                    v2 = v.AfterFirst ( ':' ) ;
                    v2 = "http://www.ncbi.nlm.nih.gov/entrez/viewer.fcgi?db=protein&val=" + v2 ;
                    v2 = v + " (" + v2 + ")" ;
                    }
                 else v2 = v ;
                 }
              i.desc += v2 ;
              }
           p = "/" + p ;
           i.setParam ( p , v ) ;
           }
        }

    // From / To
    for ( a = 1 ; a < va.GetCount() ; a++ )
        {
        if ( va[a].GetChar(0) != '/' )
           iterateItem ( v , i , va[a].Upper() ) ;
        }
    }

/** \brief Used for generating multiple items from a single GenBank item entry
    \param Pointer to the TVector
    \param i The TVectorItem new items should be based on
    \param l ???
    \param tag ???
*/
void TGenBank::iterateItem ( TVector *v , TVectorItem &i , wxString l , int tag )
    {
    l.Replace ( " " , "" ) ;
    int a ;
    while ( !l.IsEmpty() )
        {
        char c = l.GetChar(0) ;
        if ( c == '<' ) l = l.Mid ( 1 ) ;
        else if ( c == ',' ) l = l.Mid ( 1 ) ;
        else if ( c >= '0' && c <= '9' )
           {
           wxString from , to ;
           from = l.BeforeFirst ( '.' ) ;
           l = l.AfterFirst ( '.' ) ;
           while ( !l.IsEmpty() && ( l.GetChar(0) < '0' || l.GetChar(0) > '9' ) ) l = l.Mid ( 1 ) ;
           for ( a = 0 ; a < l.Length() && l.GetChar(a) >= '0' && l.GetChar(a) <= '9' ; a++ ) ;
           to = l.Left ( a ) ;
           l = l.Mid ( a ) ;
           long ll ;
           from.ToLong ( &ll ) ;
           i.from = ll ;
           to.ToLong ( &ll ) ;
           i.to = ll ;
           if ( i.to == 0 ) i.to = i.from ;
           if ( ( tag & TAG_COMPLEMENT ) > 0 ) i.setDirection ( -1 ) ;
           v->items.push_back ( i ) ;
           }
        else if ( l.Left ( 10 ) == _T("COMPLEMENT") )
           {
           l = l.AfterFirst ( '(' ) ;
           int b , cnt , num = 0 ;
           char c = ' ' ;
           for ( a = b = cnt = 0 ; a < l.Length() && ( cnt >= 0 || c != ')' ) ; a++ )
              {
              c = l.GetChar ( a ) ;
              if ( c == '(' ) cnt++ ;
              if ( ( c == ')' ) && cnt == 0 )
                 {
                 num++ ;
                 wxString sub = l.Mid ( b , a - b ) ;
                 int tag2 = tag ;
                 if ( ( tag & TAG_COMPLEMENT ) > 0 ) tag2 -= TAG_COMPLEMENT ;
                 else tag2 += TAG_COMPLEMENT ;
                 iterateItem ( v , i , sub , tag2 ) ;
                 b = a+1 ;
                 }
              if ( c == ')' ) cnt-- ;
              }
           l = l.Mid ( a ) ;
           }
        else if ( l.Left ( 4 ) == _T("JOIN") )
           {
           l = l.AfterFirst ( '(' ) ;
           int b , cnt , num = 0 ;
           char c = ' ' ;
           for ( a = b = cnt = 0 ; a < l.Length() && ( cnt >= 0 || c != ')' ) ; a++ )
              {
              c = l.GetChar ( a ) ;
              if ( c == '(' ) cnt++ ;
              if ( ( c == ',' || c == ')' ) && cnt == 0 )
                 {
                 num++ ;
                 wxString name = i.name ;
                 wxString newname = name + wxString::Format ( " (%d)" , num ) ;
                 wxString sub = l.Mid ( b , a - b ) ;
                 if ( num > 1 ) i.setParam ( _T("PREDECESSOR") , (name + wxString::Format ( " (%d)" , num-1 )) ) ;
                 if ( c != ')' ) i.setParam ( _T("SUCCESSOR") , (name + wxString::Format ( " (%d)" , num+1 )) ) ;
                 i.name = newname ;
                 iterateItem ( v , i , sub ) ;
                 b = a+1 ;
                 i.name = name ;
                 i.setParam ( _T("PREDECESSOR") , "" ) ;
                 i.setParam ( _T("SUCCESSOR") , "" ) ;
                 }
              if ( c == ')' ) cnt-- ;
              }
           l = l.Mid ( a ) ;
           }
        else
           {
           return ;
           }
        }
    }

/** \brief Counts the leading blanks in a string
    \param s The string
*/
int TGenBank::count_blanks ( wxString &s )
    {
    int a ;
//    for ( a = 0 ; a < s.length() && ( s.GetChar(a) == ' ' || s.GetChar(a) < 15 ) ; a++ ) ;
    for ( a = 0 ; a < s.length() && isblank[s.GetChar(a)] ; a++ ) ;
    return a ;
    }

/** \brief Removes the leading blanks from a string
    \param s The string
*/
wxString TGenBank::trim ( wxString s )
    {
    int a ;
//    for ( a = 0 ; a < s.length() && ( s.GetChar(a) == ' ' || s.GetChar(a) < 15 ) ; a++ ) ;
    for ( a = 0 ; a < s.length() && isblank[s.GetChar(a)] ; a++ ) ;
    if ( a ) return s.Mid ( a ) ;
    else return s ;
    }

/** \brief Removes the leading blanks from a string
    \param s The string (as reference)
*/
void TGenBank::itrim ( wxString &s )
    {
    int a ;
    for ( a = 0 ; a < s.length() && isblank[s.GetChar(a)] ; a++ ) ;
//    for ( a = 0 ; a < s.length() && ( s.GetChar(a) == ' ' || s.GetChar(a) < 15 ) ; a++ ) ;
    if ( a ) s = s.Mid ( a ) ;
    }

/** \brief Removes the leading and ending blanks and/or quotes from a string
    \param s The string (as reference)
*/
void TGenBank::multitrim ( wxString &s , bool quotes )
    {
    int a , b ;
//    for ( a = 0 ; a < s.length() && ( s.GetChar(a) == ' ' || s.GetChar(a) < 15 ||s.GetChar(a) == '"' ) ; a++ ) ;
//    for ( b = s.length()-1 ; b > 0 && ( s.GetChar(b) == ' ' || s.GetChar(b) < 15 ||s.GetChar(b) == '"' ) ; b-- ) ;

    for ( a = 0 ; a < s.length() && isblankorquote[s.GetChar(a)] ; a++ ) ;
    for ( b = s.length()-1 ; b > 0 && isblankorquote[s.GetChar(b)] ; b-- ) ;

    if ( a == 0 && b == s.length()-1 ) return ;
    s = s.Mid ( a , b - a + 1 ) ;
    }

/** \brief Removes the leading and ending quotes from a string
    \param s The string
*/
wxString TGenBank::trimQuotes ( wxString s )
    {
    if ( s.GetChar(0) == '"' ) s = s.Mid ( 1 ) ;
    if ( s.GetChar(s.Length()-1) == '"' ) s = s.Mid ( 0 , s.Length()-1 ) ;
    return s ;
    }

/** \brief Formats a new GenBank entry to a certein width
    \param ex The output
    \param init The GenBank "command"
    \param data The data (to be wrapped)
    \param limit The maximum line width
*/
void TGenBank::wrapit ( wxArrayString &ex , wxString init , wxString data , int limit )
    {
    wxString blanks = expand ( "" , init.Length() ) ;
    int allow = limit - blanks.length() ;
    wxString first = init ;
    while ( !data.IsEmpty() )
        {
        wxString s , t ;
        s = first ;
        first = blanks ;
        if ( allow > data.length() ) allow = data.length() ;
        t = data.substr ( 0 , allow ) ;
        data = data.substr ( allow ) ;
        s += t ;
        ex.Add ( s ) ;
        }
    }

/** \brief Sets a string to a certain width, and fills it up if necessary
    \param init The string to expand/cut
    \param to The desired length
    \param with To fill the string with, if needed
*/
wxString TGenBank::expand ( wxString init , int to , wxString with )
    {
    while ( init.length() < to ) init += with ;
    return init.substr ( 0 , to ) ;
    }

/** \brief ???
    \param pre ???
    \param q ???
*/
wxString TGenBank::quote ( wxString pre , wxString q )
    {
    int a ;
    for ( a = 0 ; a < q.length() ; a++ )
       {
       if ( q.GetChar(a) == '\n' || q.GetChar(a) == '\r' ) q.SetChar ( a , ' ' ) ;
       else if ( q.GetChar(a) == '\"' ) q.SetChar ( a , '-' ) ;
       }
    for ( a = 1 ; a < q.length() ; a++ )
       {
       if ( q.GetChar(a-1) == ' ' && q.GetChar(a) == ' ' )
          {
          q.erase ( a , 1 ) ;
          a-- ;
          }
       }
    return pre + "=\"" + q + "\"" ;
    }

/** \brief Create a GenBank-format list of lines from a TVector
    \param v The TVector
    \param ex The array of lines (as reference)
*/
void TGenBank::doExport ( TVector *v , wxArrayString &ex )
    {
    ex.Clear () ;
    int a , b ;
    wxString s , t , u ;
//    char z[1000] ;

    wxString b21 = expand ( "" , 21 ) ;

    // Vector
    t = expand ( v->getName() , 24 ) ;
//    sprintf ( z , _T("%d bp") , v->getSequenceLength() ) ;
    wxString z = wxString::Format ( _T("%d bp") , v->getSequenceLength() ) ;
    u = expand ( z , 11 ) ;
    s = _T("LOCUS       ") ;
    s += t ;
    s += u ;
    if ( v->getType() == TYPE_AMINO_ACIDS ) s += _T("AA      ") ;
    else s += _T("DNA     ") ;
    if ( v->isCircular() ) s += _T("circular ") ;
    else s += _T("linear ") ;
    ex.Add ( s ) ;

    // Definition
    wrapit ( ex , _T("DEFINITION  ") , v->getDescription() ) ;
    s = v->getParam ( _T("genbank") ) ;
    while ( !s.IsEmpty() && s[s.length()-1] == '\n' )
       s.RemoveLast() ;
    ex.Add ( s ) ;

    // Features
    ex.Add ( _T("FEATURES             Location/Qualifiers") ) ;
    for ( a = 0 ; a < v->items.size() ; a++ )
        {
        s = "     " ;
        wxString s2 = wxString ( gb_item_type[v->items[a].getType()] , *wxConvCurrent ) ;
        s += expand ( s2 , 16 ) ;
        if ( v->items[a].getDirection() == 1 )
            z = wxString::Format ( "%d..%d" , v->items[a].from , v->items[a].to ) ;
//            sprintf ( z , "%d..%d" , v->items[a].from , v->items[a].to ) ;
        else
            z = wxString::Format ( _T("complement(%d..%d)") , v->items[a].from , v->items[a].to ) ;
//            sprintf ( z , "complement(%d..%d)" , v->items[a].from , v->items[a].to ) ;
        ex.Add ( s + z ) ;
        if ( !v->items[a].name.IsEmpty() )
            wrapit ( ex , b21 , quote ( _T("/gene") , v->items[a].name ) ) ;
        if ( !v->items[a].desc.IsEmpty() )
            wrapit ( ex , b21 , quote ( _T("/note") , v->items[a].desc ) ) ;

        wxArrayString vs ;
        wxArrayString vss ;
        vss = v->items[a].getParamKeys() ;
        for ( b = 0 ; b < vss.GetCount() ; b++ )
            {
            vs.Add ( vss[b] ) ;
            if ( vs[b].GetChar(0) == '/' )
                wrapit ( ex , b21 , quote ( vs[b] , v->items[a].getParam(vs[b]) ) ) ;
            }

        }

    // Sequence
    int q[256] ;
    for ( a = 0 ; a < 256 ; a++ )
        q[a] = 0 ;

    t = v->getSequence() ;
    for ( a = 0 ; a < t.length() ; a++ )
        {
        b = t.GetChar(a) ;
        if ( b >= 'A' && b <= 'Z' ) b = b - 'A' + 'a' ;
        t.SetChar(a,b) ;
        q[b]++ ;
        }

    if ( v->getType() != TYPE_AMINO_ACIDS )
        {
        s = _T("BASE COUNT   ") ;
        z = wxString::Format ( "%6d a %6d c %6d g %6d t" , q['a'] , q['c'] , q['g'] , q['t'] ) ;
//       sprintf ( z , "%6d a %6d c %6d g %6d t" , q['a'] , q['c'] , q['g'] , q['t'] ) ;
        ex.Add ( s + z ) ;
        }
    while ( t.length() % 60 != 0 ) t += " " ;
    ex.Add ( _T("ORIGIN") ) ;
    for ( a = 0 ; a < t.length() ; a += 60 )
        {
//        sprintf ( z , "%9d" , a+1 ) ;
        s = wxString::Format ( "%9d" , a+1 ) ;
        for ( b = 0 ; b < 6 ; b++ )
            {
            s += " " ;
            s += t.substr ( a + b*10 , 10 ) ;
            }
        ex.Add ( s ) ;

        }

    // Finally
    ex.Add ( "//" ) ;
    }

/// Destructor, empty
TGenBank::~TGenBank ()
    {
    }

