/**CFile****************************************************************

  FileName    [cbaPtrAbc.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Hierarchical word-level netlist.]

  Synopsis    [Simple interface with external tools.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - November 29, 2014.]

  Revision    [$Id: cbaPtrAbc.c,v 1.00 2014/11/29 00:00:00 alanmi Exp $]

***********************************************************************/

#include "cba.h"
#include "base/abc/abc.h"
#include "map/mio/mio.h"
#include "base/main/mainInt.h"

ABC_NAMESPACE_IMPL_START

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Node type conversions.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
char * Ptr_HopToType( Abc_Obj_t * pObj )
{
    static word uTruth, uTruths6[3] = {
        ABC_CONST(0xAAAAAAAAAAAAAAAA),
        ABC_CONST(0xCCCCCCCCCCCCCCCC),
        ABC_CONST(0xF0F0F0F0F0F0F0F0),
    };
    assert( Abc_ObjIsNode(pObj) );
    uTruth = Hop_ManComputeTruth6( (Hop_Man_t *)Abc_ObjNtk(pObj)->pManFunc, (Hop_Obj_t *)pObj->pData, Abc_ObjFaninNum(pObj) );
/*
    if ( uTruth ==  0 )                           return "CBA_BOX_C0";
    if ( uTruth == ~(word)0 )                     return "CBA_BOX_C1";
    if ( uTruth ==  uTruths6[0] )                 return "CBA_BOX_BUF";
    if ( uTruth == ~uTruths6[0] )                 return "CBA_BOX_INV";
    if ( uTruth == (uTruths6[0] & uTruths6[1]) )  return "CBA_BOX_AND";
    if ( uTruth ==~(uTruths6[0] & uTruths6[1]) )  return "CBA_BOX_NAND";
    if ( uTruth == (uTruths6[0] | uTruths6[1]) )  return "CBA_BOX_OR";
    if ( uTruth ==~(uTruths6[0] | uTruths6[1]) )  return "CBA_BOX_NOR";
    if ( uTruth == (uTruths6[0] ^ uTruths6[1]) )  return "CBA_BOX_XOR";
    if ( uTruth ==~(uTruths6[0] ^ uTruths6[1]) )  return "CBA_BOX_XNOR";
*/
    if ( uTruth ==  0 )                           return "Const0T";
    if ( uTruth == ~(word)0 )                     return "Const1T";
    if ( uTruth ==  uTruths6[0] )                 return "BufT";
    if ( uTruth == ~uTruths6[0] )                 return "InvT";
    if ( uTruth == (uTruths6[0] & uTruths6[1]) )  return "AndT";
    if ( uTruth ==~(uTruths6[0] & uTruths6[1]) )  return "NandT";
    if ( uTruth == (uTruths6[0] | uTruths6[1]) )  return "OrT";
    if ( uTruth ==~(uTruths6[0] | uTruths6[1]) )  return "NorT";
    if ( uTruth == (uTruths6[0] ^ uTruths6[1]) )  return "XorT";
    if ( uTruth ==~(uTruths6[0] ^ uTruths6[1]) )  return "XnorT";
    assert( 0 );
    return NULL;
}


/**Function*************************************************************

  Synopsis    [Create Ptr from Abc_Ntk_t.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
char * Ptr_AbcObjName( Abc_Obj_t * pObj )
{
    if ( Abc_ObjIsNet(pObj) || Abc_ObjIsBox(pObj) )
        return Abc_ObjName(pObj);
    if ( Abc_ObjIsCi(pObj) || Abc_ObjIsNode(pObj) )
        return Ptr_AbcObjName(Abc_ObjFanout0(pObj));
    if ( Abc_ObjIsCo(pObj) )
        return Ptr_AbcObjName(Abc_ObjFanin0(pObj));
    assert( 0 );
    return NULL;
}
static int Ptr_CheckArray( Vec_Ptr_t * vArray )
{
    assert( Vec_PtrSize(vArray) == Vec_PtrCap(vArray) );
    return 1;
}
Vec_Ptr_t * Ptr_AbcDeriveNode( Abc_Obj_t * pObj )
{
    Abc_Obj_t * pFanin; int i;
    Vec_Ptr_t * vNode = Vec_PtrAllocExact( 2 + 2 * (1 + Abc_ObjFaninNum(pObj)) );
    assert( Abc_ObjIsNode(pObj) );
    if ( Abc_NtkHasAig(pObj->pNtk) )
        Vec_PtrPush( vNode, Ptr_HopToType(pObj) );
    else if ( Abc_NtkHasSop(pObj->pNtk) )
        Vec_PtrPush( vNode, Ptr_SopToTypeName((char *)pObj->pData) );
    else assert( 0 );
    Vec_PtrPush( vNode, Ptr_AbcObjName(pObj) );
    assert( Abc_ObjFaninNum(pObj) <= 2 );
    Abc_ObjForEachFanin( pObj, pFanin, i )
    {
        Vec_PtrPush( vNode, i ? "r" : "l" );
        Vec_PtrPush( vNode, Ptr_AbcObjName(pFanin) );
    }
    Vec_PtrPush( vNode, "o" );
    Vec_PtrPush( vNode, Ptr_AbcObjName(pObj) );
    assert( Ptr_CheckArray(vNode) );
    return vNode;
}
Vec_Ptr_t * Ptr_AbcDeriveBox( Abc_Obj_t * pObj )
{
    Abc_Obj_t * pNext; int i;
    Abc_Ntk_t * pNtk = Abc_ObjModel(pObj);
    Vec_Ptr_t * vBox = Vec_PtrAllocExact( 2 + 2 * Abc_ObjFaninNum(pObj) + 2 * Abc_ObjFanoutNum(pObj) );
    assert( Abc_ObjIsBox(pObj) );
    Vec_PtrPush( vBox, Abc_NtkName(pNtk) );
    Vec_PtrPush( vBox, Ptr_AbcObjName(pObj) );
    Abc_ObjForEachFanin( pObj, pNext, i )
    {
        Vec_PtrPush( vBox, Ptr_AbcObjName(Abc_NtkPi(pNtk, i)) );
        Vec_PtrPush( vBox, Ptr_AbcObjName(pNext) );
    }
    Abc_ObjForEachFanout( pObj, pNext, i )
    {
        Vec_PtrPush( vBox, Ptr_AbcObjName(Abc_NtkPo(pNtk, i)) );
        Vec_PtrPush( vBox, Ptr_AbcObjName(pNext) );
    }
    assert( Ptr_CheckArray(vBox) );
    return vBox;
}
Vec_Ptr_t * Ptr_AbcDeriveBoxes( Abc_Ntk_t * pNtk )
{
    Abc_Obj_t * pObj; int i;
    Vec_Ptr_t * vBoxes = Vec_PtrAllocExact( Abc_NtkBoxNum(pNtk) + Abc_NtkNodeNum(pNtk) );
    Abc_NtkForEachBox( pNtk, pObj, i )
        Vec_PtrPush( vBoxes, Ptr_AbcDeriveBox(pObj) );
    Abc_NtkForEachNode( pNtk, pObj, i )
        Vec_PtrPush( vBoxes, Ptr_AbcDeriveNode(pObj) );
    assert( Ptr_CheckArray(vBoxes) );
    return vBoxes;
}

Vec_Ptr_t * Ptr_AbcDeriveInputs( Abc_Ntk_t * pNtk )
{
    Abc_Obj_t * pObj; int i;
    Vec_Ptr_t * vSigs = Vec_PtrAllocExact( Abc_NtkPiNum(pNtk) );
    Abc_NtkForEachPi( pNtk, pObj, i )
        Vec_PtrPush( vSigs, Ptr_AbcObjName(pObj) );
    assert( Ptr_CheckArray(vSigs) );
    return vSigs;
}
Vec_Ptr_t * Ptr_AbcDeriveOutputs( Abc_Ntk_t * pNtk )
{
    Abc_Obj_t * pObj; int i;
    Vec_Ptr_t * vSigs = Vec_PtrAllocExact( Abc_NtkPoNum(pNtk) );
    Abc_NtkForEachPo( pNtk, pObj, i )
        Vec_PtrPush( vSigs, Ptr_AbcObjName(pObj) );
    assert( Ptr_CheckArray(vSigs) );
    return vSigs;
}
Vec_Ptr_t * Ptr_AbcDeriveNtk( Abc_Ntk_t * pNtk )
{
    Vec_Ptr_t * vNtk = Vec_PtrAllocExact( 5 );
    Vec_PtrPush( vNtk, Abc_NtkName(pNtk) );
    Vec_PtrPush( vNtk, Ptr_AbcDeriveInputs(pNtk) );
    Vec_PtrPush( vNtk, Ptr_AbcDeriveOutputs(pNtk) );
    Vec_PtrPush( vNtk, Vec_PtrAllocExact(0) );
    Vec_PtrPush( vNtk, Ptr_AbcDeriveBoxes(pNtk) );
    assert( Ptr_CheckArray(vNtk) );
    return vNtk;
}
Vec_Ptr_t * Ptr_AbcDeriveDes( Abc_Ntk_t * pNtk )
{
    Vec_Ptr_t * vDes;
    Abc_Ntk_t * pTemp; int i;
    vDes = Vec_PtrAllocExact( 1 + Vec_PtrSize(pNtk->pDesign->vModules) );
    Vec_PtrPush( vDes, pNtk->pDesign->pName );
    Vec_PtrForEachEntry( Abc_Ntk_t *, pNtk->pDesign->vModules, pTemp, i )
        Vec_PtrPush( vDes, Ptr_AbcDeriveNtk(pTemp) );
    assert( Ptr_CheckArray(vDes) );
    return vDes;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ptr_ManExperiment( Abc_Ntk_t * pNtk )
{
    abctime clk = Abc_Clock();
    char * pFileName = Extra_FileNameGenericAppend(pNtk->pDesign->pName, "_out.blif");
    Vec_Ptr_t * vDes = Ptr_AbcDeriveDes( pNtk );
    printf( "Converting to Ptr:  Memory = %6.3f MB  ", 1.0*Cba_PtrMemory(vDes)/(1<<20) );
    Abc_PrintTime( 1, "Time", Abc_Clock() - clk );
    Cba_PtrDumpBlif( pFileName, vDes );
    printf( "Finished writing output file \"%s\".  ", pFileName );
    Abc_PrintTime( 1, "Time", Abc_Clock() - clk );
    Cba_PtrFree( vDes );
}

/**Function*************************************************************

  Synopsis    [Create Cba_Man_t from tech-ind Ptr.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline int Ptr_NameToType( char * pSop )
{
    if ( !strcmp(pSop, "Const0T") )      return CBA_BOX_CF;
    if ( !strcmp(pSop, "Const1T") )      return CBA_BOX_CT;
    if ( !strcmp(pSop, "BufT") )         return CBA_BOX_BUF;
    if ( !strcmp(pSop, "InvT") )         return CBA_BOX_INV;
    if ( !strcmp(pSop, "AndT") )         return CBA_BOX_AND;
    if ( !strcmp(pSop, "NandT") )        return CBA_BOX_NAND;
    if ( !strcmp(pSop, "OrT") )          return CBA_BOX_OR;
    if ( !strcmp(pSop, "NorT") )         return CBA_BOX_NOR;
    if ( !strcmp(pSop, "XorT") )         return CBA_BOX_XOR;
    if ( !strcmp(pSop, "XnorT") )        return CBA_BOX_XNOR;
    return CBA_OBJ_BOX;
}
int Ptr_ManCountNtk( Vec_Ptr_t * vNtk )
{
    Vec_Ptr_t * vInputs = (Vec_Ptr_t *)Vec_PtrEntry(vNtk, 1);
    Vec_Ptr_t * vOutputs = (Vec_Ptr_t *)Vec_PtrEntry(vNtk, 2);
    Vec_Ptr_t * vNodes = (Vec_Ptr_t *)Vec_PtrEntry(vNtk, 3);
    Vec_Ptr_t * vBoxes = (Vec_Ptr_t *)Vec_PtrEntry(vNtk, 4);
    Vec_Ptr_t * vBox; int i, Counter = 0;
    assert( Vec_PtrSize(vNodes) == 0 );
    Counter += Vec_PtrSize(vInputs);
    Counter += Vec_PtrSize(vOutputs);
    Vec_PtrForEachEntry( Vec_Ptr_t *, vBoxes, vBox, i )
        Counter += Vec_PtrSize(vBox)/2;
    return Counter;
}
int Cba_BoxCountOutputs( Cba_Ntk_t * pNtk, char * pBoxNtk )
{
    int ModuleId = Cba_ManNtkFindId( pNtk->pDesign, pBoxNtk );
    if ( ModuleId == 0 )
        return 1;
    return Cba_NtkPoNumAlloc( Cba_ManNtk(pNtk->pDesign, ModuleId) );
}
int Cba_NtkDeriveFromPtr( Cba_Ntk_t * pNtk, Vec_Ptr_t * vNtk, Vec_Int_t * vMap, Vec_Int_t * vBox2Id )
{
    char * pName, * pModuleName = (char *)Vec_PtrEntry(vNtk, 0);
    Vec_Ptr_t * vInputs  = (Vec_Ptr_t *)Vec_PtrEntry(vNtk, 1);
    Vec_Ptr_t * vOutputs = (Vec_Ptr_t *)Vec_PtrEntry(vNtk, 2);
    Vec_Ptr_t * vBoxes   = (Vec_Ptr_t *)Vec_PtrEntry(vNtk, 4), * vBox;
    int i, k, iObj, iTerm, NameId;
    // start network with the given name
    NameId = Abc_NamStrFindOrAdd( pNtk->pDesign->pStrs, pModuleName, NULL );
    assert( Cba_NtkNameId(pNtk) == NameId );
    // map driven NameIds into their ObjIds for PIs
    Vec_PtrForEachEntry( char *, vInputs, pName, i )
    {
        NameId = Abc_NamStrFindOrAdd( pNtk->pDesign->pStrs, pName, NULL );
        if ( Vec_IntGetEntryFull(vMap, NameId) != -1 )
            { printf( "PI with name \"%s\" is not unique module \"%s\".\n", pName, pModuleName ); return 0; }
        iObj = Cba_ObjAlloc( pNtk, CBA_OBJ_PI, -1 );
        Cba_ObjSetName( pNtk, iObj, Abc_Var2Lit2(NameId, CBA_NAME_BIN) );
        Vec_IntSetEntryFull( vMap, NameId, iObj );
        Cba_NtkAddInfo( pNtk, Abc_Var2Lit2(NameId, 1), -1, -1 );
    }
    // map driven NameIds into their ObjIds for BOs
    Vec_IntClear( vBox2Id );
    Vec_PtrForEachEntry( Vec_Ptr_t *, vBoxes, vBox, i )
    {
        char * pBoxNtk = (char *)Vec_PtrEntry(vBox, 0);
        char * pBoxName = (char *)Vec_PtrEntry(vBox, 1);
        int nOutputs = Cba_BoxCountOutputs( pNtk, pBoxNtk );
        int nInputs = Vec_PtrSize(vBox)/2 - nOutputs - 1;
        int NtkId = Cba_ManNtkFindId( pNtk->pDesign, pBoxNtk );
        assert( Vec_PtrSize(vBox) % 2 == 0 );
        assert( nOutputs > 0 && 2*(nOutputs + 1) <= Vec_PtrSize(vBox) );
        iObj = Cba_BoxAlloc( pNtk, Ptr_NameToType(pBoxNtk), nInputs, nOutputs, NtkId );
        if ( NtkId > 0 )
            Cba_NtkSetHost( Cba_ManNtk(pNtk->pDesign, NtkId), Cba_NtkId(pNtk), iObj );
        Cba_ObjSetName( pNtk, iObj, Abc_Var2Lit2(Abc_NamStrFindOrAdd(pNtk->pDesign->pStrs, pBoxName, NULL), CBA_NAME_BIN) );
        Cba_BoxForEachBo( pNtk, iObj, iTerm, k )
        {
            pName = (char *)Vec_PtrEntry( vBox, Vec_PtrSize(vBox) - 2*(nOutputs - k) + 1 );
            NameId = Abc_NamStrFindOrAdd( pNtk->pDesign->pStrs, pName, NULL );
            if ( Vec_IntGetEntryFull(vMap, NameId) != -1 )
                { printf( "Signal \"%s\" has multiple drivers in module \"%s\".\n", pName, pModuleName ); return 0; }
            Cba_ObjSetName( pNtk, iTerm, Abc_Var2Lit2(NameId, CBA_NAME_BIN) );
            Vec_IntSetEntryFull( vMap, NameId, iTerm );
        }
        Vec_IntPush( vBox2Id, iObj );
    }
    assert( Vec_IntSize(vBox2Id) == Vec_PtrSize(vBoxes) );
    // connect BIs
    Vec_PtrForEachEntry( Vec_Ptr_t *, vBoxes, vBox, i )
    {
        iObj = Vec_IntEntry( vBox2Id, i );
        Cba_BoxForEachBi( pNtk, iObj, iTerm, k )
        {
            pName = (char *)Vec_PtrEntry( vBox, 2*(k + 1) + 1 );
            NameId = Abc_NamStrFindOrAdd( pNtk->pDesign->pStrs, pName, NULL );
            if ( Vec_IntGetEntryFull(vMap, NameId) == -1 )
                printf( "Signal \"%s\" in not driven in module \"%s\".\n", pName, pModuleName );
            Cba_ObjSetFanin( pNtk, iTerm, Vec_IntGetEntryFull(vMap, NameId) );
        }
    }
    // connect POs
    Vec_PtrForEachEntry( char *, vOutputs, pName, i )
    {
        NameId = Abc_NamStrFindOrAdd( pNtk->pDesign->pStrs, pName, NULL );
        if ( Vec_IntGetEntryFull(vMap, NameId) == -1 )
            printf( "PO with name \"%s\" in not driven in module \"%s\".\n", pName, pModuleName );
        iObj = Cba_ObjAlloc( pNtk, CBA_OBJ_PO, Vec_IntGetEntryFull(vMap, NameId) );
        Cba_NtkAddInfo( pNtk, Abc_Var2Lit2(NameId, 2), -1, -1 );
    }
    // update map
    Cba_NtkForEachCi( pNtk, iObj )
        Vec_IntSetEntryFull( vMap, Cba_ObjNameId(pNtk, iObj), -1 );
    // double check
    Vec_IntForEachEntry( vMap, iObj, i )
        assert( iObj == -1 );
    assert( Cba_NtkObjNum(pNtk) == Vec_StrCap(&pNtk->vType) );
    return 1;
}
Cba_Man_t * Cba_PtrTransformToCba( Vec_Ptr_t * vDes )
{
    char * pName = (char *)Vec_PtrEntry(vDes, 0);
    Cba_Man_t * pNew = Cba_ManAlloc( pName, Vec_PtrSize(vDes) - 1 );
    Vec_Int_t * vMap = Vec_IntStartFull( 1000 );
    Vec_Int_t * vBox2Id = Vec_IntAlloc( 1000 );
    // create interfaces
    Cba_Ntk_t * pNtk; int i;
    Cba_ManForEachNtk( pNew, pNtk, i )
    {
        Vec_Ptr_t * vNtk = (Vec_Ptr_t *)Vec_PtrEntry(vDes, i);
        Vec_Ptr_t * vInputs = (Vec_Ptr_t *)Vec_PtrEntry(vNtk, 1);
        Vec_Ptr_t * vOutputs = (Vec_Ptr_t *)Vec_PtrEntry(vNtk, 2);
        int NameId = Abc_NamStrFindOrAdd( pNew->pStrs, (char *)Vec_PtrEntry(vNtk, 0), NULL );
        Cba_NtkAlloc( pNtk, NameId, Vec_PtrSize(vInputs), Vec_PtrSize(vOutputs), Ptr_ManCountNtk(vNtk) );
        Cba_NtkStartNames( pNtk );
    }
    // parse the networks
    Cba_ManForEachNtk( pNew, pNtk, i )
    {
        Vec_Ptr_t * vNtk = (Vec_Ptr_t *)Vec_PtrEntry(vDes, i);
        if ( !Cba_NtkDeriveFromPtr( pNtk, vNtk, vMap, vBox2Id ) )
            break;
    }
    if ( i <= Cba_ManNtkNum(pNew) )
       Cba_ManFree(pNew), pNew = NULL;
    Vec_IntFree( vBox2Id );
    Vec_IntFree( vMap );
    return pNew;
}


/**Function*************************************************************

  Synopsis    [Create Ptr from mapped Cba_Man_t.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Ptr_t * Cba_NtkTransformToPtrBox( Cba_Ntk_t * p, int iBox )
{
    int i, iTerm, fUser = Cba_ObjIsBoxUser( p, iBox );
    Cba_Ntk_t * pBoxNtk = Cba_BoxNtk( p, iBox );
    Mio_Library_t * pLib = (Mio_Library_t *)p->pDesign->pMioLib;
    Mio_Gate_t * pGate = pLib ? Mio_LibraryReadGateByName( pLib, Cba_BoxNtkName(p, iBox), NULL ) : NULL;
    Vec_Ptr_t * vBox = Vec_PtrAllocExact( 2*Cba_BoxSize(p, iBox) );
    Vec_PtrPush( vBox, Cba_BoxNtkName(p, iBox) );
    Vec_PtrPush( vBox, Cba_ObjNameStr(p, iBox) );
    Cba_BoxForEachBi( p, iBox, iTerm, i )
    {
        Vec_PtrPush( vBox, fUser ? Cba_ObjNameStr(pBoxNtk, Cba_NtkPi(pBoxNtk, i)) : Mio_GateReadPinName(pGate, i) );
        Vec_PtrPush( vBox, Cba_ObjNameStr(p, iTerm) );
    }
    Cba_BoxForEachBo( p, iBox, iTerm, i )
    {
        Vec_PtrPush( vBox, fUser ? Cba_ObjNameStr(pBoxNtk, Cba_NtkPo(pBoxNtk, i)) : Mio_GateReadOutName(pGate) );
        Vec_PtrPush( vBox, Cba_ObjNameStr(p, iTerm) );
    }
    assert( Ptr_CheckArray(vBox) );
    return vBox;
}
Vec_Ptr_t * Cba_NtkTransformToPtrBoxes( Cba_Ntk_t * p )
{
    int iBox;
    Vec_Ptr_t * vBoxes = Vec_PtrAllocExact( Cba_NtkBoxNum(p) );
    Cba_NtkForEachBox( p, iBox )
        Vec_PtrPush( vBoxes, Cba_NtkTransformToPtrBox(p, iBox) );
    assert( Ptr_CheckArray(vBoxes) );
    return vBoxes;
}

Vec_Ptr_t * Cba_NtkTransformToPtrInputs( Cba_Ntk_t * p )
{
    int i, iTerm;
    Vec_Ptr_t * vSigs = Vec_PtrAllocExact( Cba_NtkPiNum(p) );
    Cba_NtkForEachPi( p, iTerm, i )
        Vec_PtrPush( vSigs, Cba_ObjNameStr(p, iTerm) );
    assert( Ptr_CheckArray(vSigs) );
    return vSigs;
}
Vec_Ptr_t * Cba_NtkTransformToPtrOutputs( Cba_Ntk_t * p )
{
    int i, iTerm;
    Vec_Ptr_t * vSigs = Vec_PtrAllocExact( Cba_NtkPoNum(p) );
    Cba_NtkForEachPo( p, iTerm, i )
        Vec_PtrPush( vSigs, Cba_ObjNameStr(p, iTerm) );
    assert( Ptr_CheckArray(vSigs) );
    return vSigs;
}
Vec_Ptr_t * Cba_NtkTransformToPtr( Cba_Ntk_t * p )
{
    Vec_Ptr_t * vNtk = Vec_PtrAllocExact(5);
    Vec_PtrPush( vNtk, Cba_NtkName(p) );
    Vec_PtrPush( vNtk, Cba_NtkTransformToPtrInputs(p) );
    Vec_PtrPush( vNtk, Cba_NtkTransformToPtrOutputs(p) );
    Vec_PtrPush( vNtk, Vec_PtrAllocExact(0) );
    Vec_PtrPush( vNtk, Cba_NtkTransformToPtrBoxes(p) );
    assert( Ptr_CheckArray(vNtk) );
    return vNtk;
}
Vec_Ptr_t * Cba_PtrDeriveFromCba( Cba_Man_t * p )
{
    Vec_Ptr_t * vDes;
    Cba_Ntk_t * pTemp; int i;
    if ( p == NULL )
        return NULL;
    if ( p->pMioLib == NULL )
    {
        printf( "Cannot transform CBA network into Ptr because it is not mapped.\n" );
        return NULL;
    }
    Cba_ManAssignInternWordNames( p );
    vDes = Vec_PtrAllocExact( 1 + Cba_ManNtkNum(p) );
    Vec_PtrPush( vDes, p->pName );
    Cba_ManForEachNtk( p, pTemp, i )
        Vec_PtrPush( vDes, Cba_NtkTransformToPtr(pTemp) );
    assert( Ptr_CheckArray(vDes) );
    return vDes;
}


////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

