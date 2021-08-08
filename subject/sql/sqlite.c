/*******************************file from delete.c**************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains C code routines that are called by the parser
** in order to generate code for DELETE FROM statements.
**
** $Id: delete.c,v 1.122 2006/02/24 02:53:50 drh Exp $
*/
#include "sqlitenew.h"
#include "keywordhash.h"

/*
** Look up every table that is named in pSrc.  If any table is not found,
** add an error message to pParse->zErrMsg and return NULL.  If all tables
** are found, return a pointer to the last table.
*/
/* Table *sqlite3SrcListLookup(Parse *pParse, SrcList *pSrc){ */
/*   Table *pTab = 0; */
/*   int i; */
/*   struct SrcList_item *pItem; */
/*   for(i=0, pItem=pSrc->a; i<pSrc->nSrc; i++, pItem++){ */
/*     pTab = sqlite3LocateTable(pParse, pItem->zName, pItem->zDatabase); */
/*     sqlite3DeleteTable(pParse->db, pItem->pTab); */
/*     pItem->pTab = pTab; */
/*     if( pTab ){ */
/*       pTab->nRef++; */
/*     } */
/*   } */
/*   return pTab; */
/* } */

/*
** Check to make sure the given table is writable.  If it is not
** writable, generate an error message and return 1.  If it is
** writable return 0;
*/
/* int sqlite3IsReadOnly(Parse *pParse, Table *pTab, int viewOk){ */
/*   if( pTab->readOnly && (pParse->db->flags & SQLITE_WriteSchema)==0 */
/*         && pParse->nested==0 ){ */
/*     sqlite3ErrorMsg(pParse, "table %s may not be modified", pTab->zName); */
/*     return 1; */
/*   } */
/* #ifndef SQLITE_OMIT_VIEW */
/*   if( !viewOk && pTab->pSelect ){ */
/*     sqlite3ErrorMsg(pParse,"cannot modify %s because it is a view",pTab->zName); */
/*     return 1; */
/*   } */
/* #endif */
/*   return 0; */
/* } */

/*
** Generate code that will open a table for reading.
*/
/* void sqlite3OpenTable( */
/*   Parse *p,       /1* Generate code into this VDBE *1/ */
/*   int iCur,       /1* The cursor number of the table *1/ */
/*   int iDb,        /1* The database index in sqlite3.aDb[] *1/ */
/*   Table *pTab,    /1* The table to be opened *1/ */
/*   int opcode      /1* OP_OpenRead or OP_OpenWrite *1/ */
/* ){ */
/*   Vdbe *v = sqlite3GetVdbe(p); */
/*   assert( opcode==OP_OpenWrite || opcode==OP_OpenRead ); */
/*   sqlite3TableLock(p, iDb, pTab->tnum, (opcode==OP_OpenWrite), pTab->zName); */
/*   sqlite3VdbeAddOp(v, OP_Integer, iDb, 0); */
/*   VdbeComment((v, "# %s", pTab->zName)); */
/*   sqlite3VdbeAddOp(v, opcode, iCur, pTab->tnum); */
/*   sqlite3VdbeAddOp(v, OP_SetNumColumns, iCur, pTab->nCol); */
/* } */


Delete* sqlite3DeleteNew(SrcList *pTabList, Expr *pWhere, Expr *pLimit, Expr *pOffset) {
    Delete* pNew = NULL;
    pNew = (Delete*) sqliteMalloc(sizeof(*pNew));
    if (pNew == NULL) {
        return NULL;
    }

    pNew->pTabList = pTabList;
    pNew->pWhere = pWhere;
    pNew->pLimit = pLimit;
    pNew->pOffset = pOffset;
    return pNew;
}

void sqlite3DeleteFree(Delete* deleteObj) {
    if (deleteObj == NULL) { return; }

    sqlite3SrcListDelete(deleteObj->pTabList);
    sqlite3ExprDelete(deleteObj->pWhere);
    sqlite3ExprDelete(deleteObj->pLimit);
    sqlite3ExprDelete(deleteObj->pOffset);
    sqliteFree(deleteObj);
}

/*
** Generate code for a DELETE FROM statement.
**
**     DELETE FROM table_wxyz WHERE a<5 AND b NOT NULL;
**                 \________/       \________________/
**                  pTabList              pWhere
*/
void sqlite3DeleteFrom(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table from which we should delete things */
  Expr *pWhere,           /* The WHERE clause.  May be null */
  Expr *pLimit,
  Expr *pOffset
){
    Delete* deleteObj = sqlite3DeleteNew(pTabList, pWhere, pLimit, pOffset);
    if (deleteObj == NULL) {
        sqlite3ErrorMsg(pParse, "sqlite3DeleteNew return NULL, may the malloc failed!");
    }

    ParsedResultItem item;
    item.sqltype = SQLTYPE_DELETE;
    item.result.deleteObj = deleteObj;
    sqlite3ParsedResultArrayAppend(&pParse->parsed, &item);
}

/*
** This routine generates VDBE code that causes a single row of a
** single table to be deleted.
**
** The VDBE must be in a particular state when this routine is called.
** These are the requirements:
**
**   1.  A read/write cursor pointing to pTab, the table containing the row
**       to be deleted, must be opened as cursor number "base".
**
**   2.  Read/write cursors for all indices of pTab must be open as
**       cursor number base+i for the i-th index.
**
**   3.  The record number of the row to be deleted must be on the top
**       of the stack.
**
** This routine pops the top of the stack to remove the record number
** and then generates code to remove both the table record and all index
** entries that point to that record.
*/
/* void sqlite3GenerateRowDelete( */
/*   sqlite3 *db,       /1* The database containing the index *1/ */
/*   Vdbe *v,           /1* Generate code into this VDBE *1/ */
/*   Table *pTab,       /1* Table containing the row to be deleted *1/ */
/*   int iCur,          /1* Cursor number for the table *1/ */
/*   int count          /1* Increment the row change counter *1/ */
/* ){ */
/*   int addr; */
/*   addr = sqlite3VdbeAddOp(v, OP_NotExists, iCur, 0); */
/*   sqlite3GenerateRowIndexDelete(v, pTab, iCur, 0); */
/*   sqlite3VdbeAddOp(v, OP_Delete, iCur, (count?OPFLAG_NCHANGE:0)); */
/*   if( count ){ */
/*     sqlite3VdbeChangeP3(v, -1, pTab->zName, P3_STATIC); */
/*   } */
/*   sqlite3VdbeJumpHere(v, addr); */
/* } */

/*
** This routine generates VDBE code that causes the deletion of all
** index entries associated with a single row of a single table.
**
** The VDBE must be in a particular state when this routine is called.
** These are the requirements:
**
**   1.  A read/write cursor pointing to pTab, the table containing the row
**       to be deleted, must be opened as cursor number "iCur".
**
**   2.  Read/write cursors for all indices of pTab must be open as
**       cursor number iCur+i for the i-th index.
**
**   3.  The "iCur" cursor must be pointing to the row that is to be
**       deleted.
*/
/* void sqlite3GenerateRowIndexDelete( */
/*   Vdbe *v,           /1* Generate code into this VDBE *1/ */
/*   Table *pTab,       /1* Table containing the row to be deleted *1/ */
/*   int iCur,          /1* Cursor number for the table *1/ */
/*   char *aIdxUsed     /1* Only delete if aIdxUsed!=0 && aIdxUsed[i]!=0 *1/ */
/* ){ */
/*   int i; */
/*   Index *pIdx; */

/*   for(i=1, pIdx=pTab->pIndex; pIdx; i++, pIdx=pIdx->pNext){ */
/*     if( aIdxUsed!=0 && aIdxUsed[i-1]==0 ) continue; */
/*     sqlite3GenerateIndexKey(v, pIdx, iCur); */
/*     sqlite3VdbeAddOp(v, OP_IdxDelete, iCur+i, 0); */
/*   } */
/* } */

/*
** Generate code that will assemble an index key and put it on the top
** of the tack.  The key with be for index pIdx which is an index on pTab.
** iCur is the index of a cursor open on the pTab table and pointing to
** the entry that needs indexing.
*/
/* void sqlite3GenerateIndexKey( */
/*   Vdbe *v,           /1* Generate code into this VDBE *1/ */
/*   Index *pIdx,       /1* The index for which to generate a key *1/ */
/*   int iCur           /1* Cursor number for the pIdx->pTable table *1/ */
/* ){ */
/*   int j; */
/*   Table *pTab = pIdx->pTable; */

/*   sqlite3VdbeAddOp(v, OP_Rowid, iCur, 0); */
/*   for(j=0; j<pIdx->nColumn; j++){ */
/*     int idx = pIdx->aiColumn[j]; */
/*     if( idx==pTab->iPKey ){ */
/*       sqlite3VdbeAddOp(v, OP_Dup, j, 0); */
/*     }else{ */
/*       sqlite3VdbeAddOp(v, OP_Column, iCur, idx); */
/*       sqlite3ColumnDefault(v, pTab, idx); */
/*     } */
/*   } */
/*   sqlite3VdbeAddOp(v, OP_MakeIdxRec, pIdx->nColumn, 0); */
/*   sqlite3IndexAffinityStr(v, pIdx); */
/* } */
/***************************files from insert.c***********************************/
/** This file contains C code routines that are called by the parser
** to handle INSERT statements in SQLite.
**
** $Id: insert.c,v 1.164 2006/03/15 16:26:10 drh Exp $
*/
static int selectReadsTable(Select *p, Schema *pSchema, int iTab){
  int i;
  struct SrcList_item *pItem;
  if( p->pSrc==0 ) return 0;
  for(i=0, pItem=p->pSrc->a; i<p->pSrc->nSrc; i++, pItem++){
    if( pItem->pSelect ){
      if( selectReadsTable(pItem->pSelect, pSchema, iTab) ) return 1;
    }else{
      if( pItem->pTab->pSchema==pSchema && pItem->pTab->tnum==iTab ) return 1;
    }
  }
  return 0;
}
Insert* sqlite3InsertNew(SrcList *pTabList, ExprList *pSetList, ValuesList *pValuesList, Select *pSelect, IdList *pColumn, int onError) {
    Insert *pNew = NULL;
    pNew = (Insert*)sqliteMalloc(sizeof(*pNew));
    if (pNew == NULL) {
        return NULL;
    }

    pNew->pTabList = pTabList;
    pNew->pValuesList = pValuesList;
    pNew->pSelect = pSelect;
    pNew->pColumn = pColumn;
    pNew->onError = onError;
    pNew->pSetList = pSetList; 
    return pNew;   
}

void sqlite3InsertDelete(Insert* insertObj) {
    if (insertObj) {
        sqlite3SrcListDelete(insertObj->pTabList);
        sqlite3ExprListDelete(insertObj->pSetList);
        sqlite3ValuesListDelete(insertObj->pValuesList);
        sqlite3SelectDelete(insertObj->pSelect);
        sqlite3IdListDelete(insertObj->pColumn);
        sqliteFree(insertObj);
    }
}

void sqlite3Insert(
  Parse *pParse,        /* Parser context */
  SrcList *pTabList,    /* Name of table into which we are inserting */
  ExprList *pSetList,      /* List of values to be inserted, e.g. INSERT INTO test SET id =1; */
  ValuesList *pValuesList, /* List of values*/
  Select *pSelect,      /* A SELECT statement to use as the data source */
  IdList *pColumn,      /* Column names corresponding to IDLIST. */
  int onError           /* How to handle constraint errors */
){
    Insert *insertObj = sqlite3InsertNew(pTabList, pSetList, pValuesList, pSelect, pColumn, onError);
    if (insertObj == NULL) {
        sqlite3ErrorMsg(pParse, "sqlite3InsertNew return NULL, may the malloc failed!");
    }
    ParsedResultItem item;
    item.sqltype = onError == OE_Replace ? SQLTYPE_REPLACE : SQLTYPE_INSERT;
    item.result.insertObj = insertObj;
    sqlite3ParsedResultArrayAppend(&pParse->parsed, &item);
}
/**********************************from the file select.c**********************************/
/** This file contains C code routines that are called by the parser
** to handle SELECT statements in SQLite.
**
** $Id: select.c,v 1.310 2006/03/26 01:21:23 drh Exp $
*/


/*
** Delete all the content of a Select structure but do not deallocate
** the select structure itself.
*/
static void clearSelect(Select *p){
  sqlite3ExprListDelete(p->pEList);
  sqlite3SrcListDelete(p->pSrc);
  sqlite3ExprDelete(p->pWhere);
  sqlite3ExprListDelete(p->pGroupBy);
  sqlite3ExprDelete(p->pHaving);
  sqlite3ExprListDelete(p->pOrderBy);
  sqlite3SelectDelete(p->pPrior);
  sqlite3ExprDelete(p->pLimit);
  sqlite3ExprDelete(p->pOffset);
}


/*
** Allocate a new Select structure and return a pointer to that
** structure.
*/
Select *sqlite3SelectNew(
  ExprList *pEList,     /* which columns to include in the result */
  SrcList *pSrc,        /* the FROM clause -- which tables to scan */
  Expr *pWhere,         /* the WHERE clause */
  ExprList *pGroupBy,   /* the GROUP BY clause */
  Expr *pHaving,        /* the HAVING clause */
  ExprList *pOrderBy,   /* the ORDER BY clause */
  int isDistinct,       /* true if the DISTINCT keyword is present */
  Expr *pLimit,         /* LIMIT value.  NULL means not used */
  Expr *pOffset         /* OFFSET value.  NULL means no offset */
){
  Select *pNew;
  Select standin;
  pNew = sqliteMalloc( sizeof(*pNew) );
  assert( !pOffset || pLimit );   /* Can't have OFFSET without LIMIT. */
  if( pNew==0 ){
    pNew = &standin;
    memset(pNew, 0, sizeof(*pNew));
  }
  if( pEList==0 ){
    pEList = sqlite3ExprListAppend(0, sqlite3Expr(TK_ALL,0,0,0), 0);
  }
  pNew->pEList = pEList;
  pNew->pSrc = pSrc;
  pNew->pWhere = pWhere;
  pNew->pGroupBy = pGroupBy;
  pNew->pHaving = pHaving;
  pNew->pOrderBy = pOrderBy;
  pNew->isDistinct = isDistinct;
  pNew->op = TK_SELECT;
  pNew->pLimit = pLimit;
  pNew->pOffset = pOffset;
  pNew->iLimit = -1;
  pNew->iOffset = -1;
  pNew->addrOpenVirt[0] = -1;
  pNew->addrOpenVirt[1] = -1;
  pNew->addrOpenVirt[2] = -1;
  if( pNew==&standin) {
    clearSelect(pNew);
    pNew = 0;
  }
  return pNew;
}

/*
** Delete the given Select structure and all of its substructures.
*/
void sqlite3SelectDelete(Select *p){
  if( p ){
    clearSelect(p);
    sqliteFree(p);
  }
}

/*
** Given 1 to 3 identifiers preceeding the JOIN keyword, determine the
** type of join.  Return an integer constant that expresses that type
** in terms of the following bit values:
**
**     JT_INNER
**     JT_CROSS
**     JT_OUTER
**     JT_NATURAL
**     JT_LEFT
**     JT_RIGHT
**
** A full outer join is the combination of JT_LEFT and JT_RIGHT.
**
** If an illegal or unsupported join type is seen, then still return
** a join type, but put an error in the pParse structure.
*/
int sqlite3JoinType(Parse *pParse, Token *pA, Token *pB, Token *pC){
  int jointype = 0;
  Token *apAll[3];
  Token *p;
  static const struct {
    const char zKeyword[8];
    u8 nChar;
    u8 code;
  } keywords[] = {
    { "natural", 7, JT_NATURAL },
    { "left",    4, JT_LEFT|JT_OUTER },
    { "right",   5, JT_RIGHT|JT_OUTER },
    { "full",    4, JT_FULL },
    { "outer",   5, JT_OUTER },
    { "inner",   5, JT_INNER },
    { "cross",   5, JT_INNER|JT_CROSS },
  };
  int i, j;
  apAll[0] = pA;
  apAll[1] = pB;
  apAll[2] = pC;
  for(i=0; i<3 && apAll[i]; i++){
    p = apAll[i];
    for(j=0; j<sizeof(keywords)/sizeof(keywords[0]); j++){
      if( p->n==keywords[j].nChar 
          && sqlite3StrNICmp((char*)p->z, keywords[j].zKeyword, p->n)==0 ){
        jointype |= keywords[j].code;
        break;
      }
    }
    if( j>=sizeof(keywords)/sizeof(keywords[0]) ){
      jointype |= JT_ERROR;
      break;
    }
  }
  if(
     (jointype & (JT_INNER|JT_OUTER))==(JT_INNER|JT_OUTER) ||
     (jointype & JT_ERROR)!=0
  ){
    const char *zSp1 = " ";
    const char *zSp2 = " ";
    if( pB==0 ){ zSp1++; }
    if( pC==0 ){ zSp2++; }
    sqlite3ErrorMsg(pParse, "unknown or unsupported join type: "
       "%T%s%T%s%T", pA, zSp1, pB, zSp2, pC);
    jointype = JT_INNER;
  } /* else if( jointype & JT_RIGHT ){
    sqlite3ErrorMsg(pParse, 
      "RIGHT and FULL OUTER JOINs are not currently supported");
    jointype = JT_INNER;
  }*/
  return jointype;
}

/*
** Return the index of a column in a table.  Return -1 if the column
** is not contained in the table.
*/
static int columnIndex(Table *pTab, const char *zCol){
  int i;
  for(i=0; i<pTab->nCol; i++){
    if( sqlite3StrICmp(pTab->aCol[i].zName, zCol)==0 ) return i;
  }
  return -1;
}

/*
** Set the value of a token to a '\000'-terminated string.
*/
static void setToken(Token *p, const char *z){
  p->z = (u8*)z;
  p->n = z ? strlen(z) : 0;
  p->dyn = 0;
}

/*
** Create an expression node for an identifier with the name of zName
*/
static Expr *createIdExpr(const char *zName){
  Token dummy;
  setToken(&dummy, zName);
  return sqlite3Expr(TK_ID, 0, 0, &dummy);
}


/*
** Add a term to the WHERE expression in *ppExpr that requires the
** zCol column to be equal in the two tables pTab1 and pTab2.
*/
static void addWhereTerm(
  const char *zCol,        /* Name of the column */
  const Table *pTab1,      /* First table */
  const char *zAlias1,     /* Alias for first table.  May be NULL */
  const Table *pTab2,      /* Second table */
  const char *zAlias2,     /* Alias for second table.  May be NULL */
  int iRightJoinTable,     /* VDBE cursor for the right table */
  Expr **ppExpr            /* Add the equality term to this expression */
){
  Expr *pE1a, *pE1b, *pE1c;
  Expr *pE2a, *pE2b, *pE2c;
  Expr *pE;

  pE1a = createIdExpr(zCol);
  pE2a = createIdExpr(zCol);
  if( zAlias1==0 ){
    zAlias1 = pTab1->zName;
  }
  pE1b = createIdExpr(zAlias1);
  if( zAlias2==0 ){
    zAlias2 = pTab2->zName;
  }
  pE2b = createIdExpr(zAlias2);
  pE1c = sqlite3Expr(TK_DOT, pE1b, pE1a, 0);
  pE2c = sqlite3Expr(TK_DOT, pE2b, pE2a, 0);
  pE = sqlite3Expr(TK_EQ, pE1c, pE2c, 0);
  ExprSetProperty(pE, EP_FromJoin);
  pE->iRightJoinTable = iRightJoinTable;
  *ppExpr = sqlite3ExprAnd(*ppExpr, pE);
}

/*
** Set the EP_FromJoin property on all terms of the given expression.
** And set the Expr.iRightJoinTable to iTable for every term in the
** expression.
**
** The EP_FromJoin property is used on terms of an expression to tell
** the LEFT OUTER JOIN processing logic that this term is part of the
** join restriction specified in the ON or USING clause and not a part
** of the more general WHERE clause.  These terms are moved over to the
** WHERE clause during join processing but we need to remember that they
** originated in the ON or USING clause.
**
** The Expr.iRightJoinTable tells the WHERE clause processing that the
** expression depends on table iRightJoinTable even if that table is not
** explicitly mentioned in the expression.  That information is needed
** for cases like this:
**
**    SELECT * FROM t1 LEFT JOIN t2 ON t1.a=t2.b AND t1.x=5
**
** The where clause needs to defer the handling of the t1.x=5
** term until after the t2 loop of the join.  In that way, a
** NULL t2 row will be inserted whenever t1.x!=5.  If we do not
** defer the handling of t1.x=5, it will be processed immediately
** after the t1 loop and rows with t1.x!=5 will never appear in
** the output, which is incorrect.
*/
static void setJoinExpr(Expr *p, int iTable){
  while( p ){
    ExprSetProperty(p, EP_FromJoin);
    p->iRightJoinTable = iTable;
    setJoinExpr(p->pLeft, iTable);
    p = p->pRight;
  } 
}

/*
** This routine processes the join information for a SELECT statement.
** ON and USING clauses are converted into extra terms of the WHERE clause.
** NATURAL joins also create extra WHERE clause terms.
**
** The terms of a FROM clause are contained in the Select.pSrc structure.
** The left most table is the first entry in Select.pSrc.  The right-most
** table is the last entry.  The join operator is held in the entry to
** the left.  Thus entry 0 contains the join operator for the join between
** entries 0 and 1.  Any ON or USING clauses associated with the join are
** also attached to the left entry.
**
** This routine returns the number of errors encountered.
*/
static int sqliteProcessJoin(Parse *pParse, Select *p){
  SrcList *pSrc;                  /* All tables in the FROM clause */
  int i, j;                       /* Loop counters */
  struct SrcList_item *pLeft;     /* Left table being joined */
  struct SrcList_item *pRight;    /* Right table being joined */

  pSrc = p->pSrc;
  pLeft = &pSrc->a[0];
  pRight = &pLeft[1];
  for(i=0; i<pSrc->nSrc-1; i++, pRight++, pLeft++){
    Table *pLeftTab = pLeft->pTab;
    Table *pRightTab = pRight->pTab;

    if( pLeftTab==0 || pRightTab==0 ) continue;

    /* When the NATURAL keyword is present, add WHERE clause terms for
    ** every column that the two tables have in common.
    */
    if( pLeft->jointype & JT_NATURAL ){
      if( pLeft->pOn || pLeft->pUsing ){
        sqlite3ErrorMsg(pParse, "a NATURAL join may not have "
           "an ON or USING clause", 0);
        return 1;
      }
      for(j=0; j<pLeftTab->nCol; j++){
        char *zName = pLeftTab->aCol[j].zName;
        if( columnIndex(pRightTab, zName)>=0 ){
          addWhereTerm(zName, pLeftTab, pLeft->zAlias, 
                              pRightTab, pRight->zAlias,
                              pRight->iCursor, &p->pWhere);
          
        }
      }
    }

    /* Disallow both ON and USING clauses in the same join
    */
    if( pLeft->pOn && pLeft->pUsing ){
      sqlite3ErrorMsg(pParse, "cannot have both ON and USING "
        "clauses in the same join");
      return 1;
    }

    /* Add the ON clause to the end of the WHERE clause, connected by
    ** an AND operator.
    */
    if( pLeft->pOn ){
      setJoinExpr(pLeft->pOn, pRight->iCursor);
      p->pWhere = sqlite3ExprAnd(p->pWhere, pLeft->pOn);
      pLeft->pOn = 0;
    }

    /* Create extra terms on the WHERE clause for each column named
    ** in the USING clause.  Example: If the two tables to be joined are 
    ** A and B and the USING clause names X, Y, and Z, then add this
    ** to the WHERE clause:    A.X=B.X AND A.Y=B.Y AND A.Z=B.Z
    ** Report an error if any column mentioned in the USING clause is
    ** not contained in both tables to be joined.
    */
    if( pLeft->pUsing ){
      IdList *pList = pLeft->pUsing;
      for(j=0; j<pList->nId; j++){
        char *zName = pList->a[j].zName;
        if( columnIndex(pLeftTab, zName)<0 || columnIndex(pRightTab, zName)<0 ){
          sqlite3ErrorMsg(pParse, "cannot join using column %s - column "
            "not present in both tables", zName);
          return 1;
        }
        addWhereTerm(zName, pLeftTab, pLeft->zAlias, 
                            pRightTab, pRight->zAlias,
                            pRight->iCursor, &p->pWhere);
      }
    }
  }
  return 0;
}

/*
** Insert code into "v" that will push the record on the top of the
** stack into the sorter.
*/
/* static void pushOntoSorter( */
/*   Parse *pParse,         /1* Parser context *1/ */
/*   ExprList *pOrderBy,    /1* The ORDER BY clause *1/ */
/*   Select *pSelect        /1* The whole SELECT statement *1/ */
/* ){ */
/*   Vdbe *v = pParse->pVdbe; */
/*   sqlite3ExprCodeExprList(pParse, pOrderBy); */
/*   sqlite3VdbeAddOp(v, OP_Sequence, pOrderBy->iECursor, 0); */
/*   sqlite3VdbeAddOp(v, OP_Pull, pOrderBy->nExpr + 1, 0); */
/*   sqlite3VdbeAddOp(v, OP_MakeRecord, pOrderBy->nExpr + 2, 0); */
/*   sqlite3VdbeAddOp(v, OP_IdxInsert, pOrderBy->iECursor, 0); */
/*   if( pSelect->iLimit>=0 ){ */
/*     int addr1, addr2; */
/*     addr1 = sqlite3VdbeAddOp(v, OP_IfMemZero, pSelect->iLimit+1, 0); */
/*     sqlite3VdbeAddOp(v, OP_MemIncr, -1, pSelect->iLimit+1); */
/*     addr2 = sqlite3VdbeAddOp(v, OP_Goto, 0, 0); */
/*     sqlite3VdbeJumpHere(v, addr1); */
/*     sqlite3VdbeAddOp(v, OP_Last, pOrderBy->iECursor, 0); */
/*     sqlite3VdbeAddOp(v, OP_Delete, pOrderBy->iECursor, 0); */
/*     sqlite3VdbeJumpHere(v, addr2); */
/*     pSelect->iLimit = -1; */
/*   } */
/* } */

/*
** Add code to implement the OFFSET
*/
/* static void codeOffset( */
/*   Vdbe *v,          /1* Generate code into this VM *1/ */
/*   Select *p,        /1* The SELECT statement being coded *1/ */
/*   int iContinue,    /1* Jump here to skip the current record *1/ */
/*   int nPop          /1* Number of times to pop stack when jumping *1/ */
/* ){ */
/*   if( p->iOffset>=0 && iContinue!=0 ){ */
/*     int addr; */
/*     sqlite3VdbeAddOp(v, OP_MemIncr, -1, p->iOffset); */
/*     addr = sqlite3VdbeAddOp(v, OP_IfMemNeg, p->iOffset, 0); */
/*     if( nPop>0 ){ */
/*       sqlite3VdbeAddOp(v, OP_Pop, nPop, 0); */
/*     } */
/*     sqlite3VdbeAddOp(v, OP_Goto, 0, iContinue); */
/*     VdbeComment((v, "# skip OFFSET records")); */
/*     sqlite3VdbeJumpHere(v, addr); */
/*   } */
/* } */

/*
** Add code that will check to make sure the top N elements of the
** stack are distinct.  iTab is a sorting index that holds previously
** seen combinations of the N values.  A new entry is made in iTab
** if the current N values are new.
**
** A jump to addrRepeat is made and the N+1 values are popped from the
** stack if the top N elements are not distinct.
*/
/* static void codeDistinct( */
/*   Vdbe *v,           /1* Generate code into this VM *1/ */
/*   int iTab,          /1* A sorting index used to test for distinctness *1/ */
/*   int addrRepeat,    /1* Jump to here if not distinct *1/ */
/*   int N              /1* The top N elements of the stack must be distinct *1/ */
/* ){ */
/*   sqlite3VdbeAddOp(v, OP_MakeRecord, -N, 0); */
/*   sqlite3VdbeAddOp(v, OP_Distinct, iTab, sqlite3VdbeCurrentAddr(v)+3); */
/*   sqlite3VdbeAddOp(v, OP_Pop, N+1, 0); */
/*   sqlite3VdbeAddOp(v, OP_Goto, 0, addrRepeat); */
/*   VdbeComment((v, "# skip indistinct records")); */
/*   sqlite3VdbeAddOp(v, OP_IdxInsert, iTab, 0); */
/* } */


/*
** This routine generates the code for the inside of the inner loop
** of a SELECT.
**
** If srcTab and nColumn are both zero, then the pEList expressions
** are evaluated in order to get the data for this row.  If nColumn>0
** then data is pulled from srcTab and pEList is used only to get the
** datatypes for each column.
*/
/* static int selectInnerLoop( */
/*   Parse *pParse,          /1* The parser context *1/ */
/*   Select *p,              /1* The complete select statement being coded *1/ */
/*   ExprList *pEList,       /1* List of values being extracted *1/ */
/*   int srcTab,             /1* Pull data from this table *1/ */
/*   int nColumn,            /1* Number of columns in the source table *1/ */
/*   ExprList *pOrderBy,     /1* If not NULL, sort results using this key *1/ */
/*   int distinct,           /1* If >=0, make sure results are distinct *1/ */
/*   int eDest,              /1* How to dispose of the results *1/ */
/*   int iParm,              /1* An argument to the disposal method *1/ */
/*   int iContinue,          /1* Jump here to continue with next row *1/ */
/*   int iBreak,             /1* Jump here to break out of the inner loop *1/ */
/*   char *aff               /1* affinity string if eDest is SRT_Union *1/ */
/* ){ */
/*   Vdbe *v = pParse->pVdbe; */
/*   int i; */
/*   int hasDistinct;        /1* True if the DISTINCT keyword is present *1/ */

/*   if( v==0 ) return 0; */
/*   assert( pEList!=0 ); */

/*   /1* If there was a LIMIT clause on the SELECT statement, then do the check */
/*   ** to see if this row should be output. */
/*   *1/ */
/*   hasDistinct = distinct>=0 && pEList->nExpr>0; */
/*   if( pOrderBy==0 && !hasDistinct ){ */
/*     codeOffset(v, p, iContinue, 0); */
/*   } */

/*   /1* Pull the requested columns. */
/*   *1/ */
/*   if( nColumn>0 ){ */
/*     for(i=0; i<nColumn; i++){ */
/*       sqlite3VdbeAddOp(v, OP_Column, srcTab, i); */
/*     } */
/*   }else{ */
/*     nColumn = pEList->nExpr; */
/*     sqlite3ExprCodeExprList(pParse, pEList); */
/*   } */

/*   /1* If the DISTINCT keyword was present on the SELECT statement */
/*   ** and this row has been seen before, then do not make this row */
/*   ** part of the result. */
/*   *1/ */
/*   if( hasDistinct ){ */
/*     assert( pEList!=0 ); */
/*     assert( pEList->nExpr==nColumn ); */
/*     codeDistinct(v, distinct, iContinue, nColumn); */
/*     if( pOrderBy==0 ){ */
/*       codeOffset(v, p, iContinue, nColumn); */
/*     } */
/*   } */

/*   switch( eDest ){ */
/*     /1* In this mode, write each query result to the key of the temporary */
/*     ** table iParm. */
/*     *1/ */
/* #ifndef SQLITE_OMIT_COMPOUND_SELECT */
/*     case SRT_Union: { */
/*       sqlite3VdbeAddOp(v, OP_MakeRecord, nColumn, 0); */
/*       if( aff ){ */
/*         sqlite3VdbeChangeP3(v, -1, aff, P3_STATIC); */
/*       } */
/*       sqlite3VdbeAddOp(v, OP_IdxInsert, iParm, 0); */
/*       break; */
/*     } */

/*     /1* Construct a record from the query result, but instead of */
/*     ** saving that record, use it as a key to delete elements from */
/*     ** the temporary table iParm. */
/*     *1/ */
/*     case SRT_Except: { */
/*       int addr; */
/*       addr = sqlite3VdbeAddOp(v, OP_MakeRecord, nColumn, 0); */
/*       sqlite3VdbeChangeP3(v, -1, aff, P3_STATIC); */
/*       sqlite3VdbeAddOp(v, OP_NotFound, iParm, addr+3); */
/*       sqlite3VdbeAddOp(v, OP_Delete, iParm, 0); */
/*       break; */
/*     } */
/* #endif */

/*     /1* Store the result as data using a unique key. */
/*     *1/ */
/*     case SRT_Table: */
/*     case SRT_VirtualTab: { */
/*       sqlite3VdbeAddOp(v, OP_MakeRecord, nColumn, 0); */
/*       if( pOrderBy ){ */
/*         pushOntoSorter(pParse, pOrderBy, p); */
/*       }else{ */
/*         sqlite3VdbeAddOp(v, OP_NewRowid, iParm, 0); */
/*         sqlite3VdbeAddOp(v, OP_Pull, 1, 0); */
/*         sqlite3VdbeAddOp(v, OP_Insert, iParm, 0); */
/*       } */
/*       break; */
/*     } */

/* #ifndef SQLITE_OMIT_SUBQUERY */
/*     /1* If we are creating a set for an "expr IN (SELECT ...)" construct, */
/*     ** then there should be a single item on the stack.  Write this */
/*     ** item into the set table with bogus data. */
/*     *1/ */
/*     case SRT_Set: { */
/*       int addr1 = sqlite3VdbeCurrentAddr(v); */
/*       int addr2; */

/*       assert( nColumn==1 ); */
/*       sqlite3VdbeAddOp(v, OP_NotNull, -1, addr1+3); */
/*       sqlite3VdbeAddOp(v, OP_Pop, 1, 0); */
/*       addr2 = sqlite3VdbeAddOp(v, OP_Goto, 0, 0); */
/*       if( pOrderBy ){ */
/*         /1* At first glance you would think we could optimize out the */
/*         ** ORDER BY in this case since the order of entries in the set */
/*         ** does not matter.  But there might be a LIMIT clause, in which */
/*         ** case the order does matter *1/ */
/*         pushOntoSorter(pParse, pOrderBy, p); */
/*       }else{ */
/*         char affinity = (iParm>>16)&0xFF; */
/*         affinity = sqlite3CompareAffinity(pEList->a[0].pExpr, affinity); */
/*         sqlite3VdbeOp3(v, OP_MakeRecord, 1, 0, &affinity, 1); */
/*         sqlite3VdbeAddOp(v, OP_IdxInsert, (iParm&0x0000FFFF), 0); */
/*       } */
/*       sqlite3VdbeJumpHere(v, addr2); */
/*       break; */
/*     } */

/*     /1* If any row exist in the result set, record that fact and abort. */
/*     *1/ */
/*     case SRT_Exists: { */
/*       sqlite3VdbeAddOp(v, OP_MemInt, 1, iParm); */
/*       sqlite3VdbeAddOp(v, OP_Pop, nColumn, 0); */
/*       /1* The LIMIT clause will terminate the loop for us *1/ */
/*       break; */
/*     } */

/*     /1* If this is a scalar select that is part of an expression, then */
/*     ** store the results in the appropriate memory cell and break out */
/*     ** of the scan loop. */
/*     *1/ */
/*     case SRT_Mem: { */
/*       assert( nColumn==1 ); */
/*       if( pOrderBy ){ */
/*         pushOntoSorter(pParse, pOrderBy, p); */
/*       }else{ */
/*         sqlite3VdbeAddOp(v, OP_MemStore, iParm, 1); */
/*         /1* The LIMIT clause will jump out of the loop for us *1/ */
/*       } */
/*       break; */
/*     } */
/* #endif /1* #ifndef SQLITE_OMIT_SUBQUERY *1/ */

/*     /1* Send the data to the callback function or to a subroutine.  In the */
/*     ** case of a subroutine, the subroutine itself is responsible for */
/*     ** popping the data from the stack. */
/*     *1/ */
/*     case SRT_Subroutine: */
/*     case SRT_Callback: { */
/*       if( pOrderBy ){ */
/*         sqlite3VdbeAddOp(v, OP_MakeRecord, nColumn, 0); */
/*         pushOntoSorter(pParse, pOrderBy, p); */
/*       }else if( eDest==SRT_Subroutine ){ */
/*         sqlite3VdbeAddOp(v, OP_Gosub, 0, iParm); */
/*       }else{ */
/*         sqlite3VdbeAddOp(v, OP_Callback, nColumn, 0); */
/*       } */
/*       break; */
/*     } */

/* #if !defined(SQLITE_OMIT_TRIGGER) */
/*     /1* Discard the results.  This is used for SELECT statements inside */
/*     ** the body of a TRIGGER.  The purpose of such selects is to call */
/*     ** user-defined functions that have side effects.  We do not care */
/*     ** about the actual results of the select. */
/*     *1/ */
/*     default: { */
/*       assert( eDest==SRT_Discard ); */
/*       sqlite3VdbeAddOp(v, OP_Pop, nColumn, 0); */
/*       break; */
/*     } */
/* #endif */
/*   } */

/*   /1* Jump to the end of the loop if the LIMIT is reached. */
/*   *1/ */
/*   if( p->iLimit>=0 && pOrderBy==0 ){ */
/*     sqlite3VdbeAddOp(v, OP_MemIncr, -1, p->iLimit); */
/*     sqlite3VdbeAddOp(v, OP_IfMemZero, p->iLimit, iBreak); */
/*   } */
/*   return 0; */
/* } */

/*
** Given an expression list, generate a KeyInfo structure that records
** the collating sequence for each expression in that expression list.
**
** If the ExprList is an ORDER BY or GROUP BY clause then the resulting
** KeyInfo structure is appropriate for initializing a virtual index to
** implement that clause.  If the ExprList is the result set of a SELECT
** then the KeyInfo structure is appropriate for initializing a virtual
** index to implement a DISTINCT test.
**
** Space to hold the KeyInfo structure is obtain from malloc.  The calling
** function is responsible for seeing that this structure is eventually
** freed.  Add the KeyInfo structure to the P3 field of an opcode using
** P3_KEYINFO_HANDOFF is the usual way of dealing with this.
*/
/* static KeyInfo *keyInfoFromExprList(Parse *pParse, ExprList *pList){ */
/*   sqlite3 *db = pParse->db; */
/*   int nExpr; */
/*   KeyInfo *pInfo; */
/*   struct ExprList_item *pItem; */
/*   int i; */

/*   nExpr = pList->nExpr; */
/*   pInfo = sqliteMalloc( sizeof(*pInfo) + nExpr*(sizeof(CollSeq*)+1) ); */
/*   if( pInfo ){ */
/*     pInfo->aSortOrder = (u8*)&pInfo->aColl[nExpr]; */
/*     pInfo->nField = nExpr; */
/*     pInfo->enc = ENC(db); */
/*     for(i=0, pItem=pList->a; i<nExpr; i++, pItem++){ */
/*       CollSeq *pColl; */
/*       pColl = sqlite3ExprCollSeq(pParse, pItem->pExpr); */
/*       if( !pColl ){ */
/*         pColl = db->pDfltColl; */
/*       } */
/*       pInfo->aColl[i] = pColl; */
/*       pInfo->aSortOrder[i] = pItem->sortOrder; */
/*     } */
/*   } */
/*   return pInfo; */
/* } */


/*
** If the inner loop was generated using a non-null pOrderBy argument,
** then the results were placed in a sorter.  After the loop is terminated
** we need to run the sorter and output the results.  The following
** routine generates the code needed to do that.
*/
/* static void generateSortTail( */
/*   Parse *pParse,   /1* Parsing context *1/ */
/*   Select *p,       /1* The SELECT statement *1/ */
/*   Vdbe *v,         /1* Generate code into this VDBE *1/ */
/*   int nColumn,     /1* Number of columns of data *1/ */
/*   int eDest,       /1* Write the sorted results here *1/ */
/*   int iParm        /1* Optional parameter associated with eDest *1/ */
/* ){ */
/*   int brk = sqlite3VdbeMakeLabel(v); */
/*   int cont = sqlite3VdbeMakeLabel(v); */
/*   int addr; */
/*   int iTab; */
/*   int pseudoTab; */
/*   ExprList *pOrderBy = p->pOrderBy; */

/*   iTab = pOrderBy->iECursor; */
/*   if( eDest==SRT_Callback || eDest==SRT_Subroutine ){ */
/*     pseudoTab = pParse->nTab++; */
/*     sqlite3VdbeAddOp(v, OP_OpenPseudo, pseudoTab, 0); */
/*     sqlite3VdbeAddOp(v, OP_SetNumColumns, pseudoTab, nColumn); */
/*   } */
/*   addr = 1 + sqlite3VdbeAddOp(v, OP_Sort, iTab, brk); */
/*   codeOffset(v, p, cont, 0); */
/*   if( eDest==SRT_Callback || eDest==SRT_Subroutine ){ */
/*     sqlite3VdbeAddOp(v, OP_Integer, 1, 0); */
/*   } */
/*   sqlite3VdbeAddOp(v, OP_Column, iTab, pOrderBy->nExpr + 1); */
/*   switch( eDest ){ */
/*     case SRT_Table: */
/*     case SRT_VirtualTab: { */
/*       sqlite3VdbeAddOp(v, OP_NewRowid, iParm, 0); */
/*       sqlite3VdbeAddOp(v, OP_Pull, 1, 0); */
/*       sqlite3VdbeAddOp(v, OP_Insert, iParm, 0); */
/*       break; */
/*     } */
/* #ifndef SQLITE_OMIT_SUBQUERY */
/*     case SRT_Set: { */
/*       assert( nColumn==1 ); */
/*       sqlite3VdbeAddOp(v, OP_NotNull, -1, sqlite3VdbeCurrentAddr(v)+3); */
/*       sqlite3VdbeAddOp(v, OP_Pop, 1, 0); */
/*       sqlite3VdbeAddOp(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+3); */
/*       sqlite3VdbeOp3(v, OP_MakeRecord, 1, 0, "c", P3_STATIC); */
/*       sqlite3VdbeAddOp(v, OP_IdxInsert, (iParm&0x0000FFFF), 0); */
/*       break; */
/*     } */
/*     case SRT_Mem: { */
/*       assert( nColumn==1 ); */
/*       sqlite3VdbeAddOp(v, OP_MemStore, iParm, 1); */
/*       /1* The LIMIT clause will terminate the loop for us *1/ */
/*       break; */
/*     } */
/* #endif */
/*     case SRT_Callback: */
/*     case SRT_Subroutine: { */
/*       int i; */
/*       sqlite3VdbeAddOp(v, OP_Insert, pseudoTab, 0); */
/*       for(i=0; i<nColumn; i++){ */
/*         sqlite3VdbeAddOp(v, OP_Column, pseudoTab, i); */
/*       } */
/*       if( eDest==SRT_Callback ){ */
/*         sqlite3VdbeAddOp(v, OP_Callback, nColumn, 0); */
/*       }else{ */
/*         sqlite3VdbeAddOp(v, OP_Gosub, 0, iParm); */
/*       } */
/*       break; */
/*     } */
/*     default: { */
/*       /1* Do nothing *1/ */
/*       break; */
/*     } */
/*   } */

/*   /1* Jump to the end of the loop when the LIMIT is reached */
/*   *1/ */
/*   if( p->iLimit>=0 ){ */
/*     sqlite3VdbeAddOp(v, OP_MemIncr, -1, p->iLimit); */
/*     sqlite3VdbeAddOp(v, OP_IfMemZero, p->iLimit, brk); */
/*   } */

/*   /1* The bottom of the loop */
/*   *1/ */
/*   sqlite3VdbeResolveLabel(v, cont); */
/*   sqlite3VdbeAddOp(v, OP_Next, iTab, addr); */
/*   sqlite3VdbeResolveLabel(v, brk); */
/*   if( eDest==SRT_Callback || eDest==SRT_Subroutine ){ */
/*     sqlite3VdbeAddOp(v, OP_Close, pseudoTab, 0); */
/*   } */

/* } */

/*
** Return a pointer to a string containing the 'declaration type' of the
** expression pExpr. The string may be treated as static by the caller.
**
** The declaration type is the exact datatype definition extracted from the
** original CREATE TABLE statement if the expression is a column. The
** declaration type for a ROWID field is INTEGER. Exactly when an expression
** is considered a column can be complex in the presence of subqueries. The
** result-set expression in all of the following SELECT statements is 
** considered a column by this function.
**
**   SELECT col FROM tbl;
**   SELECT (SELECT col FROM tbl;
**   SELECT (SELECT col FROM tbl);
**   SELECT abc FROM (SELECT col AS abc FROM tbl);
** 
** The declaration type for any expression other than a column is NULL.
*/
/* static const char *columnType( */
/*   NameContext *pNC, */ 
/*   Expr *pExpr, */
/*   const char **pzOriginDb, */
/*   const char **pzOriginTab, */
/*   const char **pzOriginCol */
/* ){ */
/*   char const *zType = 0; */
/*   char const *zOriginDb = 0; */
/*   char const *zOriginTab = 0; */
/*   char const *zOriginCol = 0; */
/*   int j; */
/*   if( pExpr==0 || pNC->pSrcList==0 ) return 0; */

/*   /1* The TK_AS operator can only occur in ORDER BY, GROUP BY, HAVING, */
/*   ** and LIMIT clauses.  But pExpr originates in the result set of a */
/*   ** SELECT.  So pExpr can never contain an AS operator. */
/*   *1/ */
/*   assert( pExpr->op!=TK_AS ); */

/*   switch( pExpr->op ){ */
/*     case TK_COLUMN: { */
/*       /1* The expression is a column. Locate the table the column is being */
/*       ** extracted from in NameContext.pSrcList. This table may be real */
/*       ** database table or a subquery. */
/*       *1/ */
/*       Table *pTab = 0;            /1* Table structure column is extracted from *1/ */
/*       Select *pS = 0;             /1* Select the column is extracted from *1/ */
/*       int iCol = pExpr->iColumn;  /1* Index of column in pTab *1/ */
/*       while( pNC && !pTab ){ */
/*         SrcList *pTabList = pNC->pSrcList; */
/*         for(j=0;j<pTabList->nSrc && pTabList->a[j].iCursor!=pExpr->iTable;j++); */
/*         if( j<pTabList->nSrc ){ */
/*           pTab = pTabList->a[j].pTab; */
/*           pS = pTabList->a[j].pSelect; */
/*         }else{ */
/*           pNC = pNC->pNext; */
/*         } */
/*       } */

/*       if( pTab==0 ){ */
/*         /1* FIX ME: */
/*         ** This can occurs if you have something like "SELECT new.x;" inside */
/*         ** a trigger.  In other words, if you reference the special "new" */
/*         ** table in the result set of a select.  We do not have a good way */
/*         ** to find the actual table type, so call it "TEXT".  This is really */
/*         ** something of a bug, but I do not know how to fix it. */
/*         ** */
/*         ** This code does not produce the correct answer - it just prevents */
/*         ** a segfault.  See ticket #1229. */
/*         *1/ */
/*         zType = "TEXT"; */
/*         break; */
/*       } */

/*       assert( pTab ); */
/* #ifndef SQLITE_OMIT_SUBQUERY */
/*       if( pS ){ */
/*         /1* The "table" is actually a sub-select or a view in the FROM clause */
/*         ** of the SELECT statement. Return the declaration type and origin */
/*         ** data for the result-set column of the sub-select. */
/*         *1/ */
/*         if( iCol>=0 && iCol<pS->pEList->nExpr ){ */
/*           /1* If iCol is less than zero, then the expression requests the */
/*           ** rowid of the sub-select or view. This expression is legal (see */ 
/*           ** test case misc2.2.2) - it always evaluates to NULL. */
/*           *1/ */
/*           NameContext sNC; */
/*           Expr *p = pS->pEList->a[iCol].pExpr; */
/*           sNC.pSrcList = pS->pSrc; */
/*           sNC.pNext = 0; */
/*           sNC.pParse = pNC->pParse; */
/*           zType = columnType(&sNC, p, &zOriginDb, &zOriginTab, &zOriginCol); */ 
/*         } */
/*       }else */
/* #endif */
/*       if( pTab->pSchema ){ */
/*         /1* A real table *1/ */
/*         assert( !pS ); */
/*         if( iCol<0 ) iCol = pTab->iPKey; */
/*         assert( iCol==-1 || (iCol>=0 && iCol<pTab->nCol) ); */
/*         if( iCol<0 ){ */
/*           zType = "INTEGER"; */
/*           zOriginCol = "rowid"; */
/*         }else{ */
/*           zType = pTab->aCol[iCol].zType; */
/*           zOriginCol = pTab->aCol[iCol].zName; */
/*         } */
/*         zOriginTab = pTab->zName; */
/*         if( pNC->pParse ){ */
/*           int iDb = sqlite3SchemaToIndex(pNC->pParse->db, pTab->pSchema); */
/*           zOriginDb = pNC->pParse->db->aDb[iDb].zName; */
/*         } */
/*       } */
/*       break; */
/*     } */
/* #ifndef SQLITE_OMIT_SUBQUERY */
/*     case TK_SELECT: { */
/*       /1* The expression is a sub-select. Return the declaration type and */
/*       ** origin info for the single column in the result set of the SELECT */
/*       ** statement. */
/*       *1/ */
/*       NameContext sNC; */
/*       Select *pS = pExpr->pSelect; */
/*       Expr *p = pS->pEList->a[0].pExpr; */
/*       sNC.pSrcList = pS->pSrc; */
/*       sNC.pNext = pNC; */
/*       sNC.pParse = pNC->pParse; */
/*       zType = columnType(&sNC, p, &zOriginDb, &zOriginTab, &zOriginCol); */ 
/*       break; */
/*     } */
/* #endif */
/*   } */
  
/*   if( pzOriginDb ){ */
/*     assert( pzOriginTab && pzOriginCol ); */
/*     *pzOriginDb = zOriginDb; */
/*     *pzOriginTab = zOriginTab; */
/*     *pzOriginCol = zOriginCol; */
/*   } */
/*   return zType; */
/* } */

/*
** Generate code that will tell the VDBE the declaration types of columns
** in the result set.
*/
/* static void generateColumnTypes( */
/*   Parse *pParse,      /1* Parser context *1/ */
/*   SrcList *pTabList,  /1* List of tables *1/ */
/*   ExprList *pEList    /1* Expressions defining the result set *1/ */
/* ){ */
/*   Vdbe *v = pParse->pVdbe; */
/*   int i; */
/*   NameContext sNC; */
/*   sNC.pSrcList = pTabList; */
/*   sNC.pParse = pParse; */
/*   for(i=0; i<pEList->nExpr; i++){ */
/*     Expr *p = pEList->a[i].pExpr; */
/*     const char *zOrigDb = 0; */
/*     const char *zOrigTab = 0; */
/*     const char *zOrigCol = 0; */
/*     const char *zType = columnType(&sNC, p, &zOrigDb, &zOrigTab, &zOrigCol); */

/*     /1* The vdbe must make it's own copy of the column-type and other */ 
/*     ** column specific strings, in case the schema is reset before this */
/*     ** virtual machine is deleted. */
/*     *1/ */
/*     sqlite3VdbeSetColName(v, i, COLNAME_DECLTYPE, zType, P3_TRANSIENT); */
/*     sqlite3VdbeSetColName(v, i, COLNAME_DATABASE, zOrigDb, P3_TRANSIENT); */
/*     sqlite3VdbeSetColName(v, i, COLNAME_TABLE, zOrigTab, P3_TRANSIENT); */
/*     sqlite3VdbeSetColName(v, i, COLNAME_COLUMN, zOrigCol, P3_TRANSIENT); */
/*   } */
/* } */

/*
** Generate code that will tell the VDBE the names of columns
** in the result set.  This information is used to provide the
** azCol[] values in the callback.
*/
/* static void generateColumnNames( */
/*   Parse *pParse,      /1* Parser context *1/ */
/*   SrcList *pTabList,  /1* List of tables *1/ */
/*   ExprList *pEList    /1* Expressions defining the result set *1/ */
/* ){ */
/*   Vdbe *v = pParse->pVdbe; */
/*   int i, j; */
/*   sqlite3 *db = pParse->db; */
/*   int fullNames, shortNames; */

/* #ifndef SQLITE_OMIT_EXPLAIN */
/*   /1* If this is an EXPLAIN, skip this step *1/ */
/*   if( pParse->explain ){ */
/*     return; */
/*   } */
/* #endif */

/*   assert( v!=0 ); */
/*   if( pParse->colNamesSet || v==0 || sqlite3MallocFailed() ) return; */
/*   pParse->colNamesSet = 1; */
/*   fullNames = (db->flags & SQLITE_FullColNames)!=0; */
/*   shortNames = (db->flags & SQLITE_ShortColNames)!=0; */
/*   sqlite3VdbeSetNumCols(v, pEList->nExpr); */
/*   for(i=0; i<pEList->nExpr; i++){ */
/*     Expr *p; */
/*     p = pEList->a[i].pExpr; */
/*     if( p==0 ) continue; */
/*     if( pEList->a[i].zName ){ */
/*       char *zName = pEList->a[i].zName; */
/*       sqlite3VdbeSetColName(v, i, COLNAME_NAME, zName, strlen(zName)); */
/*       continue; */
/*     } */
/*     if( p->op==TK_COLUMN && pTabList ){ */
/*       Table *pTab; */
/*       char *zCol; */
/*       int iCol = p->iColumn; */
/*       for(j=0; j<pTabList->nSrc && pTabList->a[j].iCursor!=p->iTable; j++){} */
/*       assert( j<pTabList->nSrc ); */
/*       pTab = pTabList->a[j].pTab; */
/*       if( iCol<0 ) iCol = pTab->iPKey; */
/*       assert( iCol==-1 || (iCol>=0 && iCol<pTab->nCol) ); */
/*       if( iCol<0 ){ */
/*         zCol = "rowid"; */
/*       }else{ */
/*         zCol = pTab->aCol[iCol].zName; */
/*       } */
/*       if( !shortNames && !fullNames && p->span.z && p->span.z[0] ){ */
/*         sqlite3VdbeSetColName(v, i, COLNAME_NAME, (char*)p->span.z, p->span.n); */
/*       }else if( fullNames || (!shortNames && pTabList->nSrc>1) ){ */
/*         char *zName = 0; */
/*         char *zTab; */
 
/*         zTab = pTabList->a[j].zAlias; */
/*         if( fullNames || zTab==0 ) zTab = pTab->zName; */
/*         sqlite3SetString(&zName, zTab, ".", zCol, (char*)0); */
/*         sqlite3VdbeSetColName(v, i, COLNAME_NAME, zName, P3_DYNAMIC); */
/*       }else{ */
/*         sqlite3VdbeSetColName(v, i, COLNAME_NAME, zCol, strlen(zCol)); */
/*       } */
/*     }else if( p->span.z && p->span.z[0] ){ */
/*       sqlite3VdbeSetColName(v, i, COLNAME_NAME, (char*)p->span.z, p->span.n); */
/*       /1* sqlite3VdbeCompressSpace(v, addr); *1/ */
/*     }else{ */
/*       char zName[30]; */
/*       assert( p->op!=TK_COLUMN || pTabList==0 ); */
/*       sprintf(zName, "column%d", i+1); */
/*       sqlite3VdbeSetColName(v, i, COLNAME_NAME, zName, 0); */
/*     } */
/*   } */
/*   generateColumnTypes(pParse, pTabList, pEList); */
/* } */

#ifndef SQLITE_OMIT_COMPOUND_SELECT
/*
** Name of the connection operator, used for error messages.
*/
static const char *selectOpName(int id){
  char *z;
  switch( id ){
    case TK_ALL:       z = "UNION ALL";   break;
    case TK_INTERSECT: z = "INTERSECT";   break;
    case TK_EXCEPT:    z = "EXCEPT";      break;
    default:           z = "UNION";       break;
  }
  return z;
}
#endif /* SQLITE_OMIT_COMPOUND_SELECT */

/*
** Forward declaration
*/
static int prepSelectStmt(Parse*, Select*);

/*
** Given a SELECT statement, generate a Table structure that describes
** the result set of that SELECT.
*/
/* Table *sqlite3ResultSetOfSelect(Parse *pParse, char *zTabName, Select *pSelect){ */
/*   Table *pTab; */
/*   int i, j; */
/*   ExprList *pEList; */
/*   Column *aCol, *pCol; */

/*   while( pSelect->pPrior ) pSelect = pSelect->pPrior; */
/*   if( prepSelectStmt(pParse, pSelect) ){ */
/*     return 0; */
/*   } */
/*   if( sqlite3SelectResolve(pParse, pSelect, 0) ){ */
/*     return 0; */
/*   } */
/*   pTab = sqliteMalloc( sizeof(Table) ); */
/*   if( pTab==0 ){ */
/*     return 0; */
/*   } */
/*   pTab->nRef = 1; */
/*   pTab->zName = zTabName ? sqliteStrDup(zTabName) : 0; */
/*   pEList = pSelect->pEList; */
/*   pTab->nCol = pEList->nExpr; */
/*   assert( pTab->nCol>0 ); */
/*   pTab->aCol = aCol = sqliteMalloc( sizeof(pTab->aCol[0])*pTab->nCol ); */
/*   for(i=0, pCol=aCol; i<pTab->nCol; i++, pCol++){ */
/*     Expr *p, *pR; */
/*     char *zType; */
/*     char *zName; */
/*     char *zBasename; */
/*     CollSeq *pColl; */
/*     int cnt; */
/*     NameContext sNC; */
    
/*     /1* Get an appropriate name for the column */
/*     *1/ */
/*     p = pEList->a[i].pExpr; */
/*     assert( p->pRight==0 || p->pRight->token.z==0 || p->pRight->token.z[0]!=0 ); */
/*     if( (zName = pEList->a[i].zName)!=0 ){ */
/*       /1* If the column contains an "AS <name>" phrase, use <name> as the name *1/ */
/*       zName = sqliteStrDup(zName); */
/*     }else if( p->op==TK_DOT */ 
/*               && (pR=p->pRight)!=0 && pR->token.z && pR->token.z[0] ){ */
/*       /1* For columns of the from A.B use B as the name *1/ */
/*       zName = sqlite3MPrintf("%T", &pR->token); */
/*     }else if( p->span.z && p->span.z[0] ){ */
/*       /1* Use the original text of the column expression as its name *1/ */
/*       zName = sqlite3MPrintf("%T", &p->span); */
/*     }else{ */
/*       /1* If all else fails, make up a name *1/ */
/*       zName = sqlite3MPrintf("column%d", i+1); */
/*     } */
/*     sqlite3Dequote(zName); */
/*     if( sqlite3MallocFailed() ){ */
/*       sqliteFree(zName); */
/*       sqlite3DeleteTable(0, pTab); */
/*       return 0; */
/*     } */

/*     /1* Make sure the column name is unique.  If the name is not unique, */
/*     ** append a integer to the name so that it becomes unique. */
/*     *1/ */
/*     zBasename = zName; */
/*     for(j=cnt=0; j<i; j++){ */
/*       if( sqlite3StrICmp(aCol[j].zName, zName)==0 ){ */
/*         zName = sqlite3MPrintf("%s:%d", zBasename, ++cnt); */
/*         j = -1; */
/*         if( zName==0 ) break; */
/*       } */
/*     } */
/*     if( zBasename!=zName ){ */
/*       sqliteFree(zBasename); */
/*     } */
/*     pCol->zName = zName; */

/*     /1* Get the typename, type affinity, and collating sequence for the */
/*     ** column. */
/*     *1/ */
/*     memset(&sNC, 0, sizeof(sNC)); */
/*     sNC.pSrcList = pSelect->pSrc; */
/*     zType = sqliteStrDup(columnType(&sNC, p, 0, 0, 0)); */
/*     pCol->zType = zType; */
/*     pCol->affinity = sqlite3ExprAffinity(p); */
/*     pColl = sqlite3ExprCollSeq(pParse, p); */
/*     if( pColl ){ */
/*       pCol->zColl = sqliteStrDup(pColl->zName); */
/*     } */
/*   } */
/*   pTab->iPKey = -1; */
/*   return pTab; */
/* } */

/*
** Prepare a SELECT statement for processing by doing the following
** things:
**
**    (1)  Make sure VDBE cursor numbers have been assigned to every
**         element of the FROM clause.
**
**    (2)  Fill in the pTabList->a[].pTab fields in the SrcList that 
**         defines FROM clause.  When views appear in the FROM clause,
**         fill pTabList->a[].pSelect with a copy of the SELECT statement
**         that implements the view.  A copy is made of the view's SELECT
**         statement so that we can freely modify or delete that statement
**         without worrying about messing up the presistent representation
**         of the view.
**
**    (3)  Add terms to the WHERE clause to accomodate the NATURAL keyword
**         on joins and the ON and USING clause of joins.
**
**    (4)  Scan the list of columns in the result set (pEList) looking
**         for instances of the "*" operator or the TABLE.* operator.
**         If found, expand each "*" to be every column in every table
**         and TABLE.* to be every column in TABLE.
**
** Return 0 on success.  If there are problems, leave an error message
** in pParse and return non-zero.
*/
/* static int prepSelectStmt(Parse *pParse, Select *p){ */
/*   int i, j, k, rc; */
/*   SrcList *pTabList; */
/*   ExprList *pEList; */
/*   struct SrcList_item *pFrom; */

/*   if( p==0 || p->pSrc==0 || sqlite3MallocFailed() ){ */
/*     return 1; */
/*   } */
/*   pTabList = p->pSrc; */
/*   pEList = p->pEList; */

/*   /1* Make sure cursor numbers have been assigned to all entries in */
/*   ** the FROM clause of the SELECT statement. */
/*   *1/ */
/*   sqlite3SrcListAssignCursors(pParse, p->pSrc); */

/*   /1* Look up every table named in the FROM clause of the select.  If */
/*   ** an entry of the FROM clause is a subquery instead of a table or view, */
/*   ** then create a transient table structure to describe the subquery. */
/*   *1/ */
/*   for(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){ */
/*     Table *pTab; */
/*     if( pFrom->pTab!=0 ){ */
/*       /1* This statement has already been prepared.  There is no need */
/*       ** to go further. *1/ */
/*       assert( i==0 ); */
/*       return 0; */
/*     } */
/*     if( pFrom->zName==0 ){ */
/* #ifndef SQLITE_OMIT_SUBQUERY */
/*       /1* A sub-query in the FROM clause of a SELECT *1/ */
/*       assert( pFrom->pSelect!=0 ); */
/*       if( pFrom->zAlias==0 ){ */
/*         pFrom->zAlias = */
/*           sqlite3MPrintf("sqlite_subquery_%p_", (void*)pFrom->pSelect); */
/*       } */
/*       assert( pFrom->pTab==0 ); */
/*       pFrom->pTab = pTab = */ 
/*         sqlite3ResultSetOfSelect(pParse, pFrom->zAlias, pFrom->pSelect); */
/*       if( pTab==0 ){ */
/*         return 1; */
/*       } */
/*       /1* The isTransient flag indicates that the Table structure has been */
/*       ** dynamically allocated and may be freed at any time.  In other words, */
/*       ** pTab is not pointing to a persistent table structure that defines */
/*       ** part of the schema. *1/ */
/*       pTab->isTransient = 1; */
/* #endif */
/*     }else{ */
/*       /1* An ordinary table or view name in the FROM clause *1/ */
/*       assert( pFrom->pTab==0 ); */
/*       pFrom->pTab = pTab = */ 
/*         sqlite3LocateTable(pParse,pFrom->zName,pFrom->zDatabase); */
/*       if( pTab==0 ){ */
/*         return 1; */
/*       } */
/*       pTab->nRef++; */
/* #ifndef SQLITE_OMIT_VIEW */
/*       if( pTab->pSelect ){ */
/*         /1* We reach here if the named table is a really a view *1/ */
/*         if( sqlite3ViewGetColumnNames(pParse, pTab) ){ */
/*           return 1; */
/*         } */
/*         /1* If pFrom->pSelect!=0 it means we are dealing with a */
/*         ** view within a view.  The SELECT structure has already been */
/*         ** copied by the outer view so we can skip the copy step here */
/*         ** in the inner view. */
/*         *1/ */
/*         if( pFrom->pSelect==0 ){ */
/*           pFrom->pSelect = sqlite3SelectDup(pTab->pSelect); */
/*         } */
/*       } */
/* #endif */
/*     } */
/*   } */

/*   /1* Process NATURAL keywords, and ON and USING clauses of joins. */
/*   *1/ */
/*   if( sqliteProcessJoin(pParse, p) ) return 1; */

/*   /1* For every "*" that occurs in the column list, insert the names of */
/*   ** all columns in all tables.  And for every TABLE.* insert the names */
/*   ** of all columns in TABLE.  The parser inserted a special expression */
/*   ** with the TK_ALL operator for each "*" that it found in the column list. */
/*   ** The following code just has to locate the TK_ALL expressions and expand */
/*   ** each one to the list of all columns in all tables. */
/*   ** */
/*   ** The first loop just checks to see if there are any "*" operators */
/*   ** that need expanding. */
/*   *1/ */
/*   for(k=0; k<pEList->nExpr; k++){ */
/*     Expr *pE = pEList->a[k].pExpr; */
/*     if( pE->op==TK_ALL ) break; */
/*     if( pE->op==TK_DOT && pE->pRight && pE->pRight->op==TK_ALL */
/*          && pE->pLeft && pE->pLeft->op==TK_ID ) break; */
/*   } */
/*   rc = 0; */
/*   if( k<pEList->nExpr ){ */
/*     /* */
/*     ** If we get here it means the result set contains one or more "*" */
/*     ** operators that need to be expanded.  Loop through each expression */
/*     ** in the result set and expand them one by one. */
/*     *1/ */
/*     struct ExprList_item *a = pEList->a; */
/*     ExprList *pNew = 0; */
/*     int flags = pParse->db->flags; */
/*     int longNames = (flags & SQLITE_FullColNames)!=0 && */
/*                       (flags & SQLITE_ShortColNames)==0; */

/*     for(k=0; k<pEList->nExpr; k++){ */
/*       Expr *pE = a[k].pExpr; */
/*       if( pE->op!=TK_ALL && */
/*            (pE->op!=TK_DOT || pE->pRight==0 || pE->pRight->op!=TK_ALL) ){ */
/*         /1* This particular expression does not need to be expanded. */
/*         *1/ */
/*         pNew = sqlite3ExprListAppend(pNew, a[k].pExpr, 0); */
/*         if( pNew ){ */
/*           pNew->a[pNew->nExpr-1].zName = a[k].zName; */
/*         }else{ */
/*           rc = 1; */
/*         } */
/*         a[k].pExpr = 0; */
/*         a[k].zName = 0; */
/*       }else{ */
/*         /1* This expression is a "*" or a "TABLE.*" and needs to be */
/*         ** expanded. *1/ */
/*         int tableSeen = 0;      /1* Set to 1 when TABLE matches *1/ */
/*         char *zTName;            /1* text of name of TABLE *1/ */
/*         if( pE->op==TK_DOT && pE->pLeft ){ */
/*           zTName = sqlite3NameFromToken(&pE->pLeft->token); */
/*         }else{ */
/*           zTName = 0; */
/*         } */
/*         for(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){ */
/*           Table *pTab = pFrom->pTab; */
/*           char *zTabName = pFrom->zAlias; */
/*           if( zTabName==0 || zTabName[0]==0 ){ */ 
/*             zTabName = pTab->zName; */
/*           } */
/*           if( zTName && (zTabName==0 || zTabName[0]==0 || */ 
/*                  sqlite3StrICmp(zTName, zTabName)!=0) ){ */
/*             continue; */
/*           } */
/*           tableSeen = 1; */
/*           for(j=0; j<pTab->nCol; j++){ */
/*             Expr *pExpr, *pRight; */
/*             char *zName = pTab->aCol[j].zName; */

/*             if( i>0 ){ */
/*               struct SrcList_item *pLeft = &pTabList->a[i-1]; */
/*               if( (pLeft->jointype & JT_NATURAL)!=0 && */
/*                         columnIndex(pLeft->pTab, zName)>=0 ){ */
/*                 /1* In a NATURAL join, omit the join columns from the */ 
/*                 ** table on the right *1/ */
/*                 continue; */
/*               } */
/*               if( sqlite3IdListIndex(pLeft->pUsing, zName)>=0 ){ */
/*                 /1* In a join with a USING clause, omit columns in the */
/*                 ** using clause from the table on the right. *1/ */
/*                 continue; */
/*               } */
/*             } */
/*             pRight = sqlite3Expr(TK_ID, 0, 0, 0); */
/*             if( pRight==0 ) break; */
/*             setToken(&pRight->token, zName); */
/*             if( zTabName && (longNames || pTabList->nSrc>1) ){ */
/*               Expr *pLeft = sqlite3Expr(TK_ID, 0, 0, 0); */
/*               pExpr = sqlite3Expr(TK_DOT, pLeft, pRight, 0); */
/*               if( pExpr==0 ) break; */
/*               setToken(&pLeft->token, zTabName); */
/*               setToken(&pExpr->span, sqlite3MPrintf("%s.%s", zTabName, zName)); */
/*               pExpr->span.dyn = 1; */
/*               pExpr->token.z = 0; */
/*               pExpr->token.n = 0; */
/*               pExpr->token.dyn = 0; */
/*             }else{ */
/*               pExpr = pRight; */
/*               pExpr->span = pExpr->token; */
/*             } */
/*             if( longNames ){ */
/*               pNew = sqlite3ExprListAppend(pNew, pExpr, &pExpr->span); */
/*             }else{ */
/*               pNew = sqlite3ExprListAppend(pNew, pExpr, &pRight->token); */
/*             } */
/*           } */
/*         } */
/*         if( !tableSeen ){ */
/*           if( zTName ){ */
/*             sqlite3ErrorMsg(pParse, "no such table: %s", zTName); */
/*           }else{ */
/*             sqlite3ErrorMsg(pParse, "no tables specified"); */
/*           } */
/*           rc = 1; */
/*         } */
/*         sqliteFree(zTName); */
/*       } */
/*     } */
/*     sqlite3ExprListDelete(pEList); */
/*     p->pEList = pNew; */
/*   } */
/*   return rc; */
/* } */

#ifndef SQLITE_OMIT_COMPOUND_SELECT
/*
** This routine associates entries in an ORDER BY expression list with
** columns in a result.  For each ORDER BY expression, the opcode of
** the top-level node is changed to TK_COLUMN and the iColumn value of
** the top-level node is filled in with column number and the iTable
** value of the top-level node is filled with iTable parameter.
**
** If there are prior SELECT clauses, they are processed first.  A match
** in an earlier SELECT takes precedence over a later SELECT.
**
** Any entry that does not match is flagged as an error.  The number
** of errors is returned.
*/
/* static int matchOrderbyToColumn( */
/*   Parse *pParse,          /1* A place to leave error messages *1/ */
/*   Select *pSelect,        /1* Match to result columns of this SELECT *1/ */
/*   ExprList *pOrderBy,     /1* The ORDER BY values to match against columns *1/ */
/*   int iTable,             /1* Insert this value in iTable *1/ */
/*   int mustComplete        /1* If TRUE all ORDER BYs must match *1/ */
/* ){ */
/*   int nErr = 0; */
/*   int i, j; */
/*   ExprList *pEList; */

/*   if( pSelect==0 || pOrderBy==0 ) return 1; */
/*   if( mustComplete ){ */
/*     for(i=0; i<pOrderBy->nExpr; i++){ pOrderBy->a[i].done = 0; } */
/*   } */
/*   if( prepSelectStmt(pParse, pSelect) ){ */
/*     return 1; */
/*   } */
/*   if( pSelect->pPrior ){ */
/*     if( matchOrderbyToColumn(pParse, pSelect->pPrior, pOrderBy, iTable, 0) ){ */
/*       return 1; */
/*     } */
/*   } */
/*   pEList = pSelect->pEList; */
/*   for(i=0; i<pOrderBy->nExpr; i++){ */
/*     Expr *pE = pOrderBy->a[i].pExpr; */
/*     int iCol = -1; */
/*     if( pOrderBy->a[i].done ) continue; */
/*     if( sqlite3ExprIsInteger(pE, &iCol) ){ */
/*       if( iCol<=0 || iCol>pEList->nExpr ){ */
/*         sqlite3ErrorMsg(pParse, */
/*           "ORDER BY position %d should be between 1 and %d", */
/*           iCol, pEList->nExpr); */
/*         nErr++; */
/*         break; */
/*       } */
/*       if( !mustComplete ) continue; */
/*       iCol--; */
/*     } */
/*     for(j=0; iCol<0 && j<pEList->nExpr; j++){ */
/*       if( pEList->a[j].zName && (pE->op==TK_ID || pE->op==TK_STRING) ){ */
/*         char *zName, *zLabel; */
/*         zName = pEList->a[j].zName; */
/*         zLabel = sqlite3NameFromToken(&pE->token); */
/*         assert( zLabel!=0 ); */
/*         if( sqlite3StrICmp(zName, zLabel)==0 ){ */ 
/*           iCol = j; */
/*         } */
/*         sqliteFree(zLabel); */
/*       } */
/*       if( iCol<0 && sqlite3ExprCompare(pE, pEList->a[j].pExpr) ){ */
/*         iCol = j; */
/*       } */
/*     } */
/*     if( iCol>=0 ){ */
/*       pE->op = TK_COLUMN; */
/*       pE->iColumn = iCol; */
/*       pE->iTable = iTable; */
/*       pE->iAgg = -1; */
/*       pOrderBy->a[i].done = 1; */
/*     } */
/*     if( iCol<0 && mustComplete ){ */
/*       sqlite3ErrorMsg(pParse, */
/*         "ORDER BY term number %d does not match any result column", i+1); */
/*       nErr++; */
/*       break; */
/*     } */
/*   } */
/*   return nErr; */  
/* } */
#endif /* #ifndef SQLITE_OMIT_COMPOUND_SELECT */

/*
** Get a VDBE for the given parser context.  Create a new one if necessary.
** If an error occurs, return NULL and leave a message in pParse.
*/
/* Vdbe *sqlite3GetVdbe(Parse *pParse){ */
/*   Vdbe *v = pParse->pVdbe; */
/*   if( v==0 ){ */
/*     v = pParse->pVdbe = sqlite3VdbeCreate(pParse->db); */
/*   } */
/*   return v; */
/* } */


/*
** Compute the iLimit and iOffset fields of the SELECT based on the
** pLimit and pOffset expressions.  pLimit and pOffset hold the expressions
** that appear in the original SQL statement after the LIMIT and OFFSET
** keywords.  Or NULL if those keywords are omitted. iLimit and iOffset 
** are the integer memory register numbers for counters used to compute 
** the limit and offset.  If there is no limit and/or offset, then 
** iLimit and iOffset are negative.
**
** This routine changes the values of iLimit and iOffset only if
** a limit or offset is defined by pLimit and pOffset.  iLimit and
** iOffset should have been preset to appropriate default values
** (usually but not always -1) prior to calling this routine.
** Only if pLimit!=0 or pOffset!=0 do the limit registers get
** redefined.  The UNION ALL operator uses this property to force
** the reuse of the same limit and offset registers across multiple
** SELECT statements.
*/
/* static void computeLimitRegisters(Parse *pParse, Select *p, int iBreak){ */
/*   Vdbe *v = 0; */
/*   int iLimit = 0; */
/*   int iOffset; */
/*   int addr1, addr2; */

/*   /1* */ 
/*   ** "LIMIT -1" always shows all rows.  There is some */
/*   ** contraversy about what the correct behavior should be. */
/*   ** The current implementation interprets "LIMIT 0" to mean */
/*   ** no rows. */
/*   *1/ */
/*   if( p->pLimit ){ */
/*     p->iLimit = iLimit = pParse->nMem; */
/*     pParse->nMem += 2; */
/*     v = sqlite3GetVdbe(pParse); */
/*     if( v==0 ) return; */
/*     sqlite3ExprCode(pParse, p->pLimit); */
/*     sqlite3VdbeAddOp(v, OP_MustBeInt, 0, 0); */
/*     sqlite3VdbeAddOp(v, OP_MemStore, iLimit, 0); */
/*     VdbeComment((v, "# LIMIT counter")); */
/*     sqlite3VdbeAddOp(v, OP_IfMemZero, iLimit, iBreak); */
/*   } */
/*   if( p->pOffset ){ */
/*     p->iOffset = iOffset = pParse->nMem++; */
/*     v = sqlite3GetVdbe(pParse); */
/*     if( v==0 ) return; */
/*     sqlite3ExprCode(pParse, p->pOffset); */
/*     sqlite3VdbeAddOp(v, OP_MustBeInt, 0, 0); */
/*     sqlite3VdbeAddOp(v, OP_MemStore, iOffset, p->pLimit==0); */
/*     VdbeComment((v, "# OFFSET counter")); */
/*     addr1 = sqlite3VdbeAddOp(v, OP_IfMemPos, iOffset, 0); */
/*     sqlite3VdbeAddOp(v, OP_Pop, 1, 0); */
/*     sqlite3VdbeAddOp(v, OP_Integer, 0, 0); */
/*     sqlite3VdbeJumpHere(v, addr1); */
/*     if( p->pLimit ){ */
/*       sqlite3VdbeAddOp(v, OP_Add, 0, 0); */
/*     } */
/*   } */
/*   if( p->pLimit ){ */
/*     addr1 = sqlite3VdbeAddOp(v, OP_IfMemPos, iLimit, 0); */
/*     sqlite3VdbeAddOp(v, OP_Pop, 1, 0); */
/*     sqlite3VdbeAddOp(v, OP_MemInt, -1, iLimit+1); */
/*     addr2 = sqlite3VdbeAddOp(v, OP_Goto, 0, 0); */
/*     sqlite3VdbeJumpHere(v, addr1); */
/*     sqlite3VdbeAddOp(v, OP_MemStore, iLimit+1, 1); */
/*     VdbeComment((v, "# LIMIT+OFFSET")); */
/*     sqlite3VdbeJumpHere(v, addr2); */
/*   } */
/* } */

/*
** Allocate a virtual index to use for sorting.
*/
/* static void createSortingIndex(Parse *pParse, Select *p, ExprList *pOrderBy){ */
/*   if( pOrderBy ){ */
/*     int addr; */
/*     assert( pOrderBy->iECursor==0 ); */
/*     pOrderBy->iECursor = pParse->nTab++; */
/*     addr = sqlite3VdbeAddOp(pParse->pVdbe, OP_OpenVirtual, */
/*                             pOrderBy->iECursor, pOrderBy->nExpr+1); */
/*     assert( p->addrOpenVirt[2] == -1 ); */
/*     p->addrOpenVirt[2] = addr; */
/*   } */
/* } */

#ifndef SQLITE_OMIT_COMPOUND_SELECT
/*
** Return the appropriate collating sequence for the iCol-th column of
** the result set for the compound-select statement "p".  Return NULL if
** the column has no default collating sequence.
**
** The collating sequence for the compound select is taken from the
** left-most term of the select that has a collating sequence.
*/
/* static CollSeq *multiSelectCollSeq(Parse *pParse, Select *p, int iCol){ */
/*   CollSeq *pRet; */
/*   if( p->pPrior ){ */
/*     pRet = multiSelectCollSeq(pParse, p->pPrior, iCol); */
/*   }else{ */
/*     pRet = 0; */
/*   } */
/*   if( pRet==0 ){ */
/*     pRet = sqlite3ExprCollSeq(pParse, p->pEList->a[iCol].pExpr); */
/*   } */
/*   return pRet; */
/* } */
#endif /* SQLITE_OMIT_COMPOUND_SELECT */

#ifndef SQLITE_OMIT_COMPOUND_SELECT
/*
** This routine is called to process a query that is really the union
** or intersection of two or more separate queries.
**
** "p" points to the right-most of the two queries.  the query on the
** left is p->pPrior.  The left query could also be a compound query
** in which case this routine will be called recursively. 
**
** The results of the total query are to be written into a destination
** of type eDest with parameter iParm.
**
** Example 1:  Consider a three-way compound SQL statement.
**
**     SELECT a FROM t1 UNION SELECT b FROM t2 UNION SELECT c FROM t3
**
** This statement is parsed up as follows:
**
**     SELECT c FROM t3
**      |
**      `----->  SELECT b FROM t2
**                |
**                `------>  SELECT a FROM t1
**
** The arrows in the diagram above represent the Select.pPrior pointer.
** So if this routine is called with p equal to the t3 query, then
** pPrior will be the t2 query.  p->op will be TK_UNION in this case.
**
** Notice that because of the way SQLite parses compound SELECTs, the
** individual selects always group from left to right.
*/
/* static int multiSelect( */
/*   Parse *pParse,        /1* Parsing context *1/ */
/*   Select *p,            /1* The right-most of SELECTs to be coded *1/ */
/*   int eDest,            /1* \___  Store query results as specified *1/ */
/*   int iParm,            /1* /     by these two parameters.         *1/ */
/*   char *aff             /1* If eDest is SRT_Union, the affinity string *1/ */
/* ){ */
/*   int rc = SQLITE_OK;   /1* Success code from a subroutine *1/ */
/*   Select *pPrior;       /1* Another SELECT immediately to our left *1/ */
/*   Vdbe *v;              /1* Generate code to this VDBE *1/ */
/*   int nCol;             /1* Number of columns in the result set *1/ */
/*   ExprList *pOrderBy;   /1* The ORDER BY clause on p *1/ */
/*   int aSetP2[2];        /1* Set P2 value of these op to number of columns *1/ */
/*   int nSetP2 = 0;       /1* Number of slots in aSetP2[] used *1/ */

/*   /1* Make sure there is no ORDER BY or LIMIT clause on prior SELECTs.  Only */
/*   ** the last (right-most) SELECT in the series may have an ORDER BY or LIMIT. */
/*   *1/ */
/*   if( p==0 || p->pPrior==0 ){ */
/*     rc = 1; */
/*     goto multi_select_end; */
/*   } */
/*   pPrior = p->pPrior; */
/*   assert( pPrior->pRightmost!=pPrior ); */
/*   assert( pPrior->pRightmost==p->pRightmost ); */
/*   if( pPrior->pOrderBy ){ */
/*     sqlite3ErrorMsg(pParse,"ORDER BY clause should come after %s not before", */
/*       selectOpName(p->op)); */
/*     rc = 1; */
/*     goto multi_select_end; */
/*   } */
/*   if( pPrior->pLimit ){ */
/*     sqlite3ErrorMsg(pParse,"LIMIT clause should come after %s not before", */
/*       selectOpName(p->op)); */
/*     rc = 1; */
/*     goto multi_select_end; */
/*   } */

/*   /1* Make sure we have a valid query engine.  If not, create a new one. */
/*   *1/ */
/*   v = sqlite3GetVdbe(pParse); */
/*   if( v==0 ){ */
/*     rc = 1; */
/*     goto multi_select_end; */
/*   } */

/*   /1* Create the destination temporary table if necessary */
/*   *1/ */
/*   if( eDest==SRT_VirtualTab ){ */
/*     assert( p->pEList ); */
/*     assert( nSetP2<sizeof(aSetP2)/sizeof(aSetP2[0]) ); */
/*     aSetP2[nSetP2++] = sqlite3VdbeAddOp(v, OP_OpenVirtual, iParm, 0); */
/*     eDest = SRT_Table; */
/*   } */

/*   /1* Generate code for the left and right SELECT statements. */
/*   *1/ */
/*   pOrderBy = p->pOrderBy; */
/*   switch( p->op ){ */
/*     case TK_ALL: { */
/*       if( pOrderBy==0 ){ */
/*         int addr = 0; */
/*         assert( !pPrior->pLimit ); */
/*         pPrior->pLimit = p->pLimit; */
/*         pPrior->pOffset = p->pOffset; */
/*         rc = sqlite3Select(pParse, pPrior, eDest, iParm, 0, 0, 0, aff); */
/*         p->pLimit = 0; */
/*         p->pOffset = 0; */
/*         if( rc ){ */
/*           goto multi_select_end; */
/*         } */
/*         p->pPrior = 0; */
/*         p->iLimit = pPrior->iLimit; */
/*         p->iOffset = pPrior->iOffset; */
/*         if( p->iLimit>=0 ){ */
/*           addr = sqlite3VdbeAddOp(v, OP_IfMemZero, p->iLimit, 0); */
/*           VdbeComment((v, "# Jump ahead if LIMIT reached")); */
/*         } */
/*         rc = sqlite3Select(pParse, p, eDest, iParm, 0, 0, 0, aff); */
/*         p->pPrior = pPrior; */
/*         if( rc ){ */
/*           goto multi_select_end; */
/*         } */
/*         if( addr ){ */
/*           sqlite3VdbeJumpHere(v, addr); */
/*         } */
/*         break; */
/*       } */
/*       /1* For UNION ALL ... ORDER BY fall through to the next case *1/ */
/*     } */
/*     case TK_EXCEPT: */
/*     case TK_UNION: { */
/*       int unionTab;    /1* Cursor number of the temporary table holding result *1/ */
/*       int op = 0;      /1* One of the SRT_ operations to apply to self *1/ */
/*       int priorOp;     /1* The SRT_ operation to apply to prior selects *1/ */
/*       Expr *pLimit, *pOffset; /1* Saved values of p->nLimit and p->nOffset *1/ */
/*       int addr; */

/*       priorOp = p->op==TK_ALL ? SRT_Table : SRT_Union; */
/*       if( eDest==priorOp && pOrderBy==0 && !p->pLimit && !p->pOffset ){ */
/*         /1* We can reuse a temporary table generated by a SELECT to our */
/*         ** right. */
/*         *1/ */
/*         unionTab = iParm; */
/*       }else{ */
/*         /1* We will need to create our own temporary table to hold the */
/*         ** intermediate results. */
/*         *1/ */
/*         unionTab = pParse->nTab++; */
/*         if( pOrderBy && matchOrderbyToColumn(pParse, p, pOrderBy, unionTab,1) ){ */
/*           rc = 1; */
/*           goto multi_select_end; */
/*         } */
/*         addr = sqlite3VdbeAddOp(v, OP_OpenVirtual, unionTab, 0); */
/*         if( priorOp==SRT_Table ){ */
/*           assert( nSetP2<sizeof(aSetP2)/sizeof(aSetP2[0]) ); */
/*           aSetP2[nSetP2++] = addr; */
/*         }else{ */
/*           assert( p->addrOpenVirt[0] == -1 ); */
/*           p->addrOpenVirt[0] = addr; */
/*           p->pRightmost->usesVirt = 1; */
/*         } */
/*         createSortingIndex(pParse, p, pOrderBy); */
/*         assert( p->pEList ); */
/*       } */

/*       /1* Code the SELECT statements to our left */
/*       *1/ */
/*       assert( !pPrior->pOrderBy ); */
/*       rc = sqlite3Select(pParse, pPrior, priorOp, unionTab, 0, 0, 0, aff); */
/*       if( rc ){ */
/*         goto multi_select_end; */
/*       } */

/*       /1* Code the current SELECT statement */
/*       *1/ */
/*       switch( p->op ){ */
/*          case TK_EXCEPT:  op = SRT_Except;   break; */
/*          case TK_UNION:   op = SRT_Union;    break; */
/*          case TK_ALL:     op = SRT_Table;    break; */
/*       } */
/*       p->pPrior = 0; */
/*       p->pOrderBy = 0; */
/*       p->disallowOrderBy = pOrderBy!=0; */
/*       pLimit = p->pLimit; */
/*       p->pLimit = 0; */
/*       pOffset = p->pOffset; */
/*       p->pOffset = 0; */
/*       rc = sqlite3Select(pParse, p, op, unionTab, 0, 0, 0, aff); */
/*       p->pPrior = pPrior; */
/*       p->pOrderBy = pOrderBy; */
/*       sqlite3ExprDelete(p->pLimit); */
/*       p->pLimit = pLimit; */
/*       p->pOffset = pOffset; */
/*       p->iLimit = -1; */
/*       p->iOffset = -1; */
/*       if( rc ){ */
/*         goto multi_select_end; */
/*       } */


/*       /1* Convert the data in the temporary table into whatever form */
/*       ** it is that we currently need. */
/*       *1/ */      
/*       if( eDest!=priorOp || unionTab!=iParm ){ */
/*         int iCont, iBreak, iStart; */
/*         assert( p->pEList ); */
/*         if( eDest==SRT_Callback ){ */
/*           Select *pFirst = p; */
/*           while( pFirst->pPrior ) pFirst = pFirst->pPrior; */
/*           generateColumnNames(pParse, 0, pFirst->pEList); */
/*         } */
/*         iBreak = sqlite3VdbeMakeLabel(v); */
/*         iCont = sqlite3VdbeMakeLabel(v); */
/*         computeLimitRegisters(pParse, p, iBreak); */
/*         sqlite3VdbeAddOp(v, OP_Rewind, unionTab, iBreak); */
/*         iStart = sqlite3VdbeCurrentAddr(v); */
/*         rc = selectInnerLoop(pParse, p, p->pEList, unionTab, p->pEList->nExpr, */
/*                              pOrderBy, -1, eDest, iParm, */ 
/*                              iCont, iBreak, 0); */
/*         if( rc ){ */
/*           rc = 1; */
/*           goto multi_select_end; */
/*         } */
/*         sqlite3VdbeResolveLabel(v, iCont); */
/*         sqlite3VdbeAddOp(v, OP_Next, unionTab, iStart); */
/*         sqlite3VdbeResolveLabel(v, iBreak); */
/*         sqlite3VdbeAddOp(v, OP_Close, unionTab, 0); */
/*       } */
/*       break; */
/*     } */
/*     case TK_INTERSECT: { */
/*       int tab1, tab2; */
/*       int iCont, iBreak, iStart; */
/*       Expr *pLimit, *pOffset; */
/*       int addr; */

/*       /1* INTERSECT is different from the others since it requires */
/*       ** two temporary tables.  Hence it has its own case.  Begin */
/*       ** by allocating the tables we will need. */
/*       *1/ */
/*       tab1 = pParse->nTab++; */
/*       tab2 = pParse->nTab++; */
/*       if( pOrderBy && matchOrderbyToColumn(pParse,p,pOrderBy,tab1,1) ){ */
/*         rc = 1; */
/*         goto multi_select_end; */
/*       } */
/*       createSortingIndex(pParse, p, pOrderBy); */

/*       addr = sqlite3VdbeAddOp(v, OP_OpenVirtual, tab1, 0); */
/*       assert( p->addrOpenVirt[0] == -1 ); */
/*       p->addrOpenVirt[0] = addr; */
/*       p->pRightmost->usesVirt = 1; */
/*       assert( p->pEList ); */

/*       /1* Code the SELECTs to our left into temporary table "tab1". */
/*       *1/ */
/*       rc = sqlite3Select(pParse, pPrior, SRT_Union, tab1, 0, 0, 0, aff); */
/*       if( rc ){ */
/*         goto multi_select_end; */
/*       } */

/*       /1* Code the current SELECT into temporary table "tab2" */
/*       *1/ */
/*       addr = sqlite3VdbeAddOp(v, OP_OpenVirtual, tab2, 0); */
/*       assert( p->addrOpenVirt[1] == -1 ); */
/*       p->addrOpenVirt[1] = addr; */
/*       p->pPrior = 0; */
/*       pLimit = p->pLimit; */
/*       p->pLimit = 0; */
/*       pOffset = p->pOffset; */
/*       p->pOffset = 0; */
/*       rc = sqlite3Select(pParse, p, SRT_Union, tab2, 0, 0, 0, aff); */
/*       p->pPrior = pPrior; */
/*       sqlite3ExprDelete(p->pLimit); */
/*       p->pLimit = pLimit; */
/*       p->pOffset = pOffset; */
/*       if( rc ){ */
/*         goto multi_select_end; */
/*       } */

/*       /1* Generate code to take the intersection of the two temporary */
/*       ** tables. */
/*       *1/ */
/*       assert( p->pEList ); */
/*       if( eDest==SRT_Callback ){ */
/*         Select *pFirst = p; */
/*         while( pFirst->pPrior ) pFirst = pFirst->pPrior; */
/*         generateColumnNames(pParse, 0, pFirst->pEList); */
/*       } */
/*       iBreak = sqlite3VdbeMakeLabel(v); */
/*       iCont = sqlite3VdbeMakeLabel(v); */
/*       computeLimitRegisters(pParse, p, iBreak); */
/*       sqlite3VdbeAddOp(v, OP_Rewind, tab1, iBreak); */
/*       iStart = sqlite3VdbeAddOp(v, OP_RowKey, tab1, 0); */
/*       sqlite3VdbeAddOp(v, OP_NotFound, tab2, iCont); */
/*       rc = selectInnerLoop(pParse, p, p->pEList, tab1, p->pEList->nExpr, */
/*                              pOrderBy, -1, eDest, iParm, */ 
/*                              iCont, iBreak, 0); */
/*       if( rc ){ */
/*         rc = 1; */
/*         goto multi_select_end; */
/*       } */
/*       sqlite3VdbeResolveLabel(v, iCont); */
/*       sqlite3VdbeAddOp(v, OP_Next, tab1, iStart); */
/*       sqlite3VdbeResolveLabel(v, iBreak); */
/*       sqlite3VdbeAddOp(v, OP_Close, tab2, 0); */
/*       sqlite3VdbeAddOp(v, OP_Close, tab1, 0); */
/*       break; */
/*     } */
/*   } */

/*   /1* Make sure all SELECTs in the statement have the same number of elements */
/*   ** in their result sets. */
/*   *1/ */
/*   assert( p->pEList && pPrior->pEList ); */
/*   if( p->pEList->nExpr!=pPrior->pEList->nExpr ){ */
/*     sqlite3ErrorMsg(pParse, "SELECTs to the left and right of %s" */
/*       " do not have the same number of result columns", selectOpName(p->op)); */
/*     rc = 1; */
/*     goto multi_select_end; */
/*   } */

/*   /1* Set the number of columns in temporary tables */
/*   *1/ */
/*   nCol = p->pEList->nExpr; */
/*   while( nSetP2 ){ */
/*     sqlite3VdbeChangeP2(v, aSetP2[--nSetP2], nCol); */
/*   } */

/*   /1* Compute collating sequences used by either the ORDER BY clause or */
/*   ** by any temporary tables needed to implement the compound select. */
/*   ** Attach the KeyInfo structure to all temporary tables.  Invoke the */
/*   ** ORDER BY processing if there is an ORDER BY clause. */
/*   ** */
/*   ** This section is run by the right-most SELECT statement only. */
/*   ** SELECT statements to the left always skip this part.  The right-most */
/*   ** SELECT might also skip this part if it has no ORDER BY clause and */
/*   ** no temp tables are required. */
/*   *1/ */
/*   if( pOrderBy || p->usesVirt ){ */
/*     int i;                        /1* Loop counter *1/ */
/*     KeyInfo *pKeyInfo;            /1* Collating sequence for the result set *1/ */
/*     Select *pLoop;                /1* For looping through SELECT statements *1/ */
/*     CollSeq **apColl; */
/*     CollSeq **aCopy; */

/*     assert( p->pRightmost==p ); */
/*     pKeyInfo = sqliteMalloc(sizeof(*pKeyInfo)+nCol*2*sizeof(CollSeq*) + nCol); */
/*     if( !pKeyInfo ){ */
/*       rc = SQLITE_NOMEM; */
/*       goto multi_select_end; */
/*     } */

/*     pKeyInfo->enc = ENC(pParse->db); */
/*     pKeyInfo->nField = nCol; */

/*     for(i=0, apColl=pKeyInfo->aColl; i<nCol; i++, apColl++){ */
/*       *apColl = multiSelectCollSeq(pParse, p, i); */
/*       if( 0==*apColl ){ */
/*         *apColl = pParse->db->pDfltColl; */
/*       } */
/*     } */

/*     for(pLoop=p; pLoop; pLoop=pLoop->pPrior){ */
/*       for(i=0; i<2; i++){ */
/*         int addr = pLoop->addrOpenVirt[i]; */
/*         if( addr<0 ){ */
/*           /1* If [0] is unused then [1] is also unused.  So we can */
/*           ** always safely abort as soon as the first unused slot is found *1/ */
/*           assert( pLoop->addrOpenVirt[1]<0 ); */
/*           break; */
/*         } */
/*         sqlite3VdbeChangeP2(v, addr, nCol); */
/*         sqlite3VdbeChangeP3(v, addr, (char*)pKeyInfo, P3_KEYINFO); */
/*       } */
/*     } */

/*     if( pOrderBy ){ */
/*       struct ExprList_item *pOTerm = pOrderBy->a; */
/*       int nOrderByExpr = pOrderBy->nExpr; */
/*       int addr; */
/*       u8 *pSortOrder; */

/*       aCopy = &pKeyInfo->aColl[nCol]; */
/*       pSortOrder = pKeyInfo->aSortOrder = (u8*)&aCopy[nCol]; */
/*       memcpy(aCopy, pKeyInfo->aColl, nCol*sizeof(CollSeq*)); */
/*       apColl = pKeyInfo->aColl; */
/*       for(i=0; i<nOrderByExpr; i++, pOTerm++, apColl++, pSortOrder++){ */
/*         Expr *pExpr = pOTerm->pExpr; */
/*         char *zName = pOTerm->zName; */
/*         assert( pExpr->op==TK_COLUMN && pExpr->iColumn<nCol ); */
/*         if( zName ){ */
/*           *apColl = sqlite3LocateCollSeq(pParse, zName, -1); */
/*         }else{ */
/*           *apColl = aCopy[pExpr->iColumn]; */
/*         } */
/*         *pSortOrder = pOTerm->sortOrder; */
/*       } */
/*       assert( p->pRightmost==p ); */
/*       assert( p->addrOpenVirt[2]>=0 ); */
/*       addr = p->addrOpenVirt[2]; */
/*       sqlite3VdbeChangeP2(v, addr, p->pEList->nExpr+2); */
/*       pKeyInfo->nField = nOrderByExpr; */
/*       sqlite3VdbeChangeP3(v, addr, (char*)pKeyInfo, P3_KEYINFO_HANDOFF); */
/*       pKeyInfo = 0; */
/*       generateSortTail(pParse, p, v, p->pEList->nExpr, eDest, iParm); */
/*     } */

/*     sqliteFree(pKeyInfo); */
/*   } */

/* multi_select_end: */
/*   return rc; */
/* } */
#endif /* SQLITE_OMIT_COMPOUND_SELECT */

#ifndef SQLITE_OMIT_VIEW
/*
** Scan through the expression pExpr.  Replace every reference to
** a column in table number iTable with a copy of the iColumn-th
** entry in pEList.  (But leave references to the ROWID column 
** unchanged.)
**
** This routine is part of the flattening procedure.  A subquery
** whose result set is defined by pEList appears as entry in the
** FROM clause of a SELECT such that the VDBE cursor assigned to that
** FORM clause entry is iTable.  This routine make the necessary 
** changes to pExpr so that it refers directly to the source table
** of the subquery rather the result set of the subquery.
*/
static void substExprList(ExprList*,int,ExprList*);  /* Forward Decl */
static void substSelect(Select *, int, ExprList *);  /* Forward Decl */
static void substExpr(Expr *pExpr, int iTable, ExprList *pEList){
  if( pExpr==0 ) return;
  if( pExpr->op==TK_COLUMN && pExpr->iTable==iTable ){
    if( pExpr->iColumn<0 ){
      pExpr->op = TK_NULL;
    }else{
      Expr *pNew;
      assert( pEList!=0 && pExpr->iColumn<pEList->nExpr );
      assert( pExpr->pLeft==0 && pExpr->pRight==0 && pExpr->pList==0 );
      pNew = pEList->a[pExpr->iColumn].pExpr;
      assert( pNew!=0 );
      pExpr->op = pNew->op;
      assert( pExpr->pLeft==0 );
      pExpr->pLeft = sqlite3ExprDup(pNew->pLeft);
      assert( pExpr->pRight==0 );
      pExpr->pRight = sqlite3ExprDup(pNew->pRight);
      assert( pExpr->pList==0 );
      pExpr->pList = sqlite3ExprListDup(pNew->pList);
      pExpr->iTable = pNew->iTable;
      pExpr->iColumn = pNew->iColumn;
      pExpr->iAgg = pNew->iAgg;
      sqlite3TokenCopy(&pExpr->token, &pNew->token);
      sqlite3TokenCopy(&pExpr->span, &pNew->span);
      pExpr->pSelect = sqlite3SelectDup(pNew->pSelect);
      pExpr->flags = pNew->flags;
    }
  }else{
    substExpr(pExpr->pLeft, iTable, pEList);
    substExpr(pExpr->pRight, iTable, pEList);
    substSelect(pExpr->pSelect, iTable, pEList);
    substExprList(pExpr->pList, iTable, pEList);
  }
}
static void substExprList(ExprList *pList, int iTable, ExprList *pEList){
  int i;
  if( pList==0 ) return;
  for(i=0; i<pList->nExpr; i++){
    substExpr(pList->a[i].pExpr, iTable, pEList);
  }
}
static void substSelect(Select *p, int iTable, ExprList *pEList){
  if( !p ) return;
  substExprList(p->pEList, iTable, pEList);
  substExprList(p->pGroupBy, iTable, pEList);
  substExprList(p->pOrderBy, iTable, pEList);
  substExpr(p->pHaving, iTable, pEList);
  substExpr(p->pWhere, iTable, pEList);
}
#endif /* !defined(SQLITE_OMIT_VIEW) */

#ifndef SQLITE_OMIT_VIEW
/*
** This routine attempts to flatten subqueries in order to speed
** execution.  It returns 1 if it makes changes and 0 if no flattening
** occurs.
**
** To understand the concept of flattening, consider the following
** query:
**
**     SELECT a FROM (SELECT x+y AS a FROM t1 WHERE z<100) WHERE a>5
**
** The default way of implementing this query is to execute the
** subquery first and store the results in a temporary table, then
** run the outer query on that temporary table.  This requires two
** passes over the data.  Furthermore, because the temporary table
** has no indices, the WHERE clause on the outer query cannot be
** optimized.
**
** This routine attempts to rewrite queries such as the above into
** a single flat select, like this:
**
**     SELECT x+y AS a FROM t1 WHERE z<100 AND a>5
**
** The code generated for this simpification gives the same result
** but only has to scan the data once.  And because indices might 
** exist on the table t1, a complete scan of the data might be
** avoided.
**
** Flattening is only attempted if all of the following are true:
**
**   (1)  The subquery and the outer query do not both use aggregates.
**
**   (2)  The subquery is not an aggregate or the outer query is not a join.
**
**   (3)  The subquery is not the right operand of a left outer join, or
**        the subquery is not itself a join.  (Ticket #306)
**
**   (4)  The subquery is not DISTINCT or the outer query is not a join.
**
**   (5)  The subquery is not DISTINCT or the outer query does not use
**        aggregates.
**
**   (6)  The subquery does not use aggregates or the outer query is not
**        DISTINCT.
**
**   (7)  The subquery has a FROM clause.
**
**   (8)  The subquery does not use LIMIT or the outer query is not a join.
**
**   (9)  The subquery does not use LIMIT or the outer query does not use
**        aggregates.
**
**  (10)  The subquery does not use aggregates or the outer query does not
**        use LIMIT.
**
**  (11)  The subquery and the outer query do not both have ORDER BY clauses.
**
**  (12)  The subquery is not the right term of a LEFT OUTER JOIN or the
**        subquery has no WHERE clause.  (added by ticket #350)
**
**  (13)  The subquery and outer query do not both use LIMIT
**
**  (14)  The subquery does not use OFFSET
**
** In this routine, the "p" parameter is a pointer to the outer query.
** The subquery is p->pSrc->a[iFrom].  isAgg is true if the outer query
** uses aggregates and subqueryIsAgg is true if the subquery uses aggregates.
**
** If flattening is not attempted, this routine is a no-op and returns 0.
** If flattening is attempted this routine returns 1.
**
** All of the expression analysis must occur on both the outer query and
** the subquery before this routine runs.
*/
/* static int flattenSubquery( */
/*   Select *p,           /1* The parent or outer SELECT statement *1/ */
/*   int iFrom,           /1* Index in p->pSrc->a[] of the inner subquery *1/ */
/*   int isAgg,           /1* True if outer SELECT uses aggregate functions *1/ */
/*   int subqueryIsAgg    /1* True if the subquery uses aggregate functions *1/ */
/* ){ */
/*   Select *pSub;       /1* The inner query or "subquery" *1/ */
/*   SrcList *pSrc;      /1* The FROM clause of the outer query *1/ */
/*   SrcList *pSubSrc;   /1* The FROM clause of the subquery *1/ */
/*   ExprList *pList;    /1* The result set of the outer query *1/ */
/*   int iParent;        /1* VDBE cursor number of the pSub result set temp table *1/ */
/*   int i;              /1* Loop counter *1/ */
/*   Expr *pWhere;                    /1* The WHERE clause *1/ */
/*   struct SrcList_item *pSubitem;   /1* The subquery *1/ */

/*   /1* Check to see if flattening is permitted.  Return 0 if not. */
/*   *1/ */
/*   if( p==0 ) return 0; */
/*   pSrc = p->pSrc; */
/*   assert( pSrc && iFrom>=0 && iFrom<pSrc->nSrc ); */
/*   pSubitem = &pSrc->a[iFrom]; */
/*   pSub = pSubitem->pSelect; */
/*   assert( pSub!=0 ); */
/*   if( isAgg && subqueryIsAgg ) return 0;                 /1* Restriction (1)  *1/ */
/*   if( subqueryIsAgg && pSrc->nSrc>1 ) return 0;          /1* Restriction (2)  *1/ */
/*   pSubSrc = pSub->pSrc; */
/*   assert( pSubSrc ); */
/*   /1* Prior to version 3.1.2, when LIMIT and OFFSET had to be simple constants, */
/*   ** not arbitrary expresssions, we allowed some combining of LIMIT and OFFSET */
/*   ** because they could be computed at compile-time.  But when LIMIT and OFFSET */
/*   ** became arbitrary expressions, we were forced to add restrictions (13) */
/*   ** and (14). *1/ */
/*   if( pSub->pLimit && p->pLimit ) return 0;              /1* Restriction (13) *1/ */
/*   if( pSub->pOffset ) return 0;                          /1* Restriction (14) *1/ */
/*   if( pSubSrc->nSrc==0 ) return 0;                       /1* Restriction (7)  *1/ */
/*   if( (pSub->isDistinct || pSub->pLimit) */ 
/*          && (pSrc->nSrc>1 || isAgg) ){          /1* Restrictions (4)(5)(8)(9) *1/ */
/*      return 0; */       
/*   } */
/*   if( p->isDistinct && subqueryIsAgg ) return 0;         /1* Restriction (6)  *1/ */
/*   if( (p->disallowOrderBy || p->pOrderBy) && pSub->pOrderBy ){ */
/*      return 0;                                           /1* Restriction (11) *1/ */
/*   } */

/*   /1* Restriction 3:  If the subquery is a join, make sure the subquery is */ 
/*   ** not used as the right operand of an outer join.  Examples of why this */
/*   ** is not allowed: */
/*   ** */
/*   **         t1 LEFT OUTER JOIN (t2 JOIN t3) */
/*   ** */
/*   ** If we flatten the above, we would get */
/*   ** */
/*   **         (t1 LEFT OUTER JOIN t2) JOIN t3 */
/*   ** */
/*   ** which is not at all the same thing. */
/*   *1/ */
/*   if( pSubSrc->nSrc>1 && iFrom>0 && (pSrc->a[iFrom-1].jointype & JT_OUTER)!=0 ){ */
/*     return 0; */
/*   } */

/*   /1* Restriction 12:  If the subquery is the right operand of a left outer */
/*   ** join, make sure the subquery has no WHERE clause. */
/*   ** An examples of why this is not allowed: */
/*   ** */
/*   **         t1 LEFT OUTER JOIN (SELECT * FROM t2 WHERE t2.x>0) */
/*   ** */
/*   ** If we flatten the above, we would get */
/*   ** */
/*   **         (t1 LEFT OUTER JOIN t2) WHERE t2.x>0 */
/*   ** */
/*   ** But the t2.x>0 test will always fail on a NULL row of t2, which */
/*   ** effectively converts the OUTER JOIN into an INNER JOIN. */
/*   *1/ */
/*   if( iFrom>0 && (pSrc->a[iFrom-1].jointype & JT_OUTER)!=0 */ 
/*       && pSub->pWhere!=0 ){ */
/*     return 0; */
/*   } */

/*   /1* If we reach this point, it means flattening is permitted for the */
/*   ** iFrom-th entry of the FROM clause in the outer query. */
/*   *1/ */

/*   /1* Move all of the FROM elements of the subquery into the */
/*   ** the FROM clause of the outer query.  Before doing this, remember */
/*   ** the cursor number for the original outer query FROM element in */
/*   ** iParent.  The iParent cursor will never be used.  Subsequent code */
/*   ** will scan expressions looking for iParent references and replace */
/*   ** those references with expressions that resolve to the subquery FROM */
/*   ** elements we are now copying in. */
/*   *1/ */
/*   iParent = pSubitem->iCursor; */
/*   { */
/*     int nSubSrc = pSubSrc->nSrc; */
/*     int jointype = pSubitem->jointype; */

/*     sqlite3DeleteTable(0, pSubitem->pTab); */
/*     sqliteFree(pSubitem->zDatabase); */
/*     sqliteFree(pSubitem->zName); */
/*     sqliteFree(pSubitem->zAlias); */
/*     if( nSubSrc>1 ){ */
/*       int extra = nSubSrc - 1; */
/*       for(i=1; i<nSubSrc; i++){ */
/*         pSrc = sqlite3SrcListAppend(pSrc, 0, 0); */
/*       } */
/*       p->pSrc = pSrc; */
/*       for(i=pSrc->nSrc-1; i-extra>=iFrom; i--){ */
/*         pSrc->a[i] = pSrc->a[i-extra]; */
/*       } */
/*     } */
/*     for(i=0; i<nSubSrc; i++){ */
/*       pSrc->a[i+iFrom] = pSubSrc->a[i]; */
/*       memset(&pSubSrc->a[i], 0, sizeof(pSubSrc->a[i])); */
/*     } */
/*     pSrc->a[iFrom+nSubSrc-1].jointype = jointype; */
/*   } */

/*   /1* Now begin substituting subquery result set expressions for */ 
/*   ** references to the iParent in the outer query. */
/*   ** */ 
/*   ** Example: */
/*   ** */
/*   **   SELECT a+5, b*10 FROM (SELECT x*3 AS a, y+10 AS b FROM t1) WHERE a>b; */
/*   **   \                     \_____________ subquery __________/          / */
/*   **    \_____________________ outer query ______________________________/ */
/*   ** */
/*   ** We look at every expression in the outer query and every place we see */
/*   ** "a" we substitute "x*3" and every place we see "b" we substitute "y+10". */
/*   *1/ */
/*   pList = p->pEList; */
/*   for(i=0; i<pList->nExpr; i++){ */
/*     Expr *pExpr; */
/*     if( pList->a[i].zName==0 && (pExpr = pList->a[i].pExpr)->span.z!=0 ){ */
/*       pList->a[i].zName = sqliteStrNDup((char*)pExpr->span.z, pExpr->span.n); */
/*     } */
/*   } */
/*   substExprList(p->pEList, iParent, pSub->pEList); */
/*   if( isAgg ){ */
/*     substExprList(p->pGroupBy, iParent, pSub->pEList); */
/*     substExpr(p->pHaving, iParent, pSub->pEList); */
/*   } */
/*   if( pSub->pOrderBy ){ */
/*     assert( p->pOrderBy==0 ); */
/*     p->pOrderBy = pSub->pOrderBy; */
/*     pSub->pOrderBy = 0; */
/*   }else if( p->pOrderBy ){ */
/*     substExprList(p->pOrderBy, iParent, pSub->pEList); */
/*   } */
/*   if( pSub->pWhere ){ */
/*     pWhere = sqlite3ExprDup(pSub->pWhere); */
/*   }else{ */
/*     pWhere = 0; */
/*   } */
/*   if( subqueryIsAgg ){ */
/*     assert( p->pHaving==0 ); */
/*     p->pHaving = p->pWhere; */
/*     p->pWhere = pWhere; */
/*     substExpr(p->pHaving, iParent, pSub->pEList); */
/*     p->pHaving = sqlite3ExprAnd(p->pHaving, sqlite3ExprDup(pSub->pHaving)); */
/*     assert( p->pGroupBy==0 ); */
/*     p->pGroupBy = sqlite3ExprListDup(pSub->pGroupBy); */
/*   }else{ */
/*     substExpr(p->pWhere, iParent, pSub->pEList); */
/*     p->pWhere = sqlite3ExprAnd(p->pWhere, pWhere); */
/*   } */

/*   /1* The flattened query is distinct if either the inner or the */
/*   ** outer query is distinct. */ 
/*   *1/ */
/*   p->isDistinct = p->isDistinct || pSub->isDistinct; */

/*   /* */
/*   ** SELECT ... FROM (SELECT ... LIMIT a OFFSET b) LIMIT x OFFSET y; */
/*   ** */
/*   ** One is tempted to try to add a and b to combine the limits.  But this */
/*   ** does not work if either limit is negative. */
/*   *1/ */
/*   if( pSub->pLimit ){ */
/*     p->pLimit = pSub->pLimit; */
/*     pSub->pLimit = 0; */
/*   } */

/*   /1* Finially, delete what is left of the subquery and return */
/*   ** success. */
/*   *1/ */
/*   sqlite3SelectDelete(pSub); */
/*   return 1; */
/* } */
#endif /* SQLITE_OMIT_VIEW */

/*
** Analyze the SELECT statement passed in as an argument to see if it
** is a simple min() or max() query.  If it is and this query can be
** satisfied using a single seek to the beginning or end of an index,
** then generate the code for this SELECT and return 1.  If this is not a 
** simple min() or max() query, then return 0;
**
** A simply min() or max() query looks like this:
**
**    SELECT min(a) FROM table;
**    SELECT max(a) FROM table;
**
** The query may have only a single table in its FROM argument.  There
** can be no GROUP BY or HAVING or WHERE clauses.  The result set must
** be the min() or max() of a single column of the table.  The column
** in the min() or max() function must be indexed.
**
** The parameters to this routine are the same as for sqlite3Select().
** See the header comment on that routine for additional information.
*/
/* static int simpleMinMaxQuery(Parse *pParse, Select *p, int eDest, int iParm){ */
/*   Expr *pExpr; */
/*   int iCol; */
/*   Table *pTab; */
/*   Index *pIdx; */
/*   int base; */
/*   Vdbe *v; */
/*   int seekOp; */
/*   ExprList *pEList, *pList, eList; */
/*   struct ExprList_item eListItem; */
/*   SrcList *pSrc; */
/*   int brk; */
/*   int iDb; */

/*   /1* Check to see if this query is a simple min() or max() query.  Return */
/*   ** zero if it is  not. */
/*   *1/ */
/*   if( p->pGroupBy || p->pHaving || p->pWhere ) return 0; */
/*   pSrc = p->pSrc; */
/*   if( pSrc->nSrc!=1 ) return 0; */
/*   pEList = p->pEList; */
/*   if( pEList->nExpr!=1 ) return 0; */
/*   pExpr = pEList->a[0].pExpr; */
/*   if( pExpr->op!=TK_AGG_FUNCTION ) return 0; */
/*   pList = pExpr->pList; */
/*   if( pList==0 || pList->nExpr!=1 ) return 0; */
/*   if( pExpr->token.n!=3 ) return 0; */
/*   if( sqlite3StrNICmp((char*)pExpr->token.z,"min",3)==0 ){ */
/*     seekOp = OP_Rewind; */
/*   }else if( sqlite3StrNICmp((char*)pExpr->token.z,"max",3)==0 ){ */
/*     seekOp = OP_Last; */
/*   }else{ */
/*     return 0; */
/*   } */
/*   pExpr = pList->a[0].pExpr; */
/*   if( pExpr->op!=TK_COLUMN ) return 0; */
/*   iCol = pExpr->iColumn; */
/*   pTab = pSrc->a[0].pTab; */


/*   /1* If we get to here, it means the query is of the correct form. */
/*   ** Check to make sure we have an index and make pIdx point to the */
/*   ** appropriate index.  If the min() or max() is on an INTEGER PRIMARY */
/*   ** key column, no index is necessary so set pIdx to NULL.  If no */
/*   ** usable index is found, return 0. */
/*   *1/ */
/*   if( iCol<0 ){ */
/*     pIdx = 0; */
/*   }else{ */
/*     CollSeq *pColl = sqlite3ExprCollSeq(pParse, pExpr); */
/*     for(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){ */
/*       assert( pIdx->nColumn>=1 ); */
/*       if( pIdx->aiColumn[0]==iCol && */ 
/*           0==sqlite3StrICmp(pIdx->azColl[0], pColl->zName) ){ */
/*         break; */
/*       } */
/*     } */
/*     if( pIdx==0 ) return 0; */
/*   } */

/*   /1* Identify column types if we will be using the callback.  This */
/*   ** step is skipped if the output is going to a table or a memory cell. */
/*   ** The column names have already been generated in the calling function. */
/*   *1/ */
/*   v = sqlite3GetVdbe(pParse); */
/*   if( v==0 ) return 0; */

/*   /1* If the output is destined for a temporary table, open that table. */
/*   *1/ */
/*   if( eDest==SRT_VirtualTab ){ */
/*     sqlite3VdbeAddOp(v, OP_OpenVirtual, iParm, 1); */
/*   } */

/*   /1* Generating code to find the min or the max.  Basically all we have */
/*   ** to do is find the first or the last entry in the chosen index.  If */
/*   ** the min() or max() is on the INTEGER PRIMARY KEY, then find the first */
/*   ** or last entry in the main table. */
/*   *1/ */
/*   iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema); */
/*   assert( iDb>=0 || pTab->isTransient ); */
/*   sqlite3CodeVerifySchema(pParse, iDb); */
/*   sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName); */
/*   base = pSrc->a[0].iCursor; */
/*   brk = sqlite3VdbeMakeLabel(v); */
/*   computeLimitRegisters(pParse, p, brk); */
/*   if( pSrc->a[0].pSelect==0 ){ */
/*     sqlite3OpenTable(pParse, base, iDb, pTab, OP_OpenRead); */
/*   } */
/*   if( pIdx==0 ){ */
/*     sqlite3VdbeAddOp(v, seekOp, base, 0); */
/*   }else{ */
/*     /1* Even though the cursor used to open the index here is closed */
/*     ** as soon as a single value has been read from it, allocate it */
/*     ** using (pParse->nTab++) to prevent the cursor id from being */ 
/*     ** reused. This is important for statements of the form */ 
/*     ** "INSERT INTO x SELECT max() FROM x". */
/*     *1/ */
/*     int iIdx; */
/*     KeyInfo *pKey = sqlite3IndexKeyinfo(pParse, pIdx); */
/*     iIdx = pParse->nTab++; */
/*     assert( pIdx->pSchema==pTab->pSchema ); */
/*     sqlite3VdbeAddOp(v, OP_Integer, iDb, 0); */
/*     sqlite3VdbeOp3(v, OP_OpenRead, iIdx, pIdx->tnum, */ 
/*         (char*)pKey, P3_KEYINFO_HANDOFF); */
/*     if( seekOp==OP_Rewind ){ */
/*       sqlite3VdbeAddOp(v, OP_Null, 0, 0); */
/*       sqlite3VdbeAddOp(v, OP_MakeRecord, 1, 0); */
/*       seekOp = OP_MoveGt; */
/*     } */
/*     sqlite3VdbeAddOp(v, seekOp, iIdx, 0); */
/*     sqlite3VdbeAddOp(v, OP_IdxRowid, iIdx, 0); */
/*     sqlite3VdbeAddOp(v, OP_Close, iIdx, 0); */
/*     sqlite3VdbeAddOp(v, OP_MoveGe, base, 0); */
/*   } */
/*   eList.nExpr = 1; */
/*   memset(&eListItem, 0, sizeof(eListItem)); */
/*   eList.a = &eListItem; */
/*   eList.a[0].pExpr = pExpr; */
/*   selectInnerLoop(pParse, p, &eList, 0, 0, 0, -1, eDest, iParm, brk, brk, 0); */
/*   sqlite3VdbeResolveLabel(v, brk); */
/*   sqlite3VdbeAddOp(v, OP_Close, base, 0); */
  
/*   return 1; */
/* } */

/*
** Analyze and ORDER BY or GROUP BY clause in a SELECT statement.  Return
** the number of errors seen.
**
** An ORDER BY or GROUP BY is a list of expressions.  If any expression
** is an integer constant, then that expression is replaced by the
** corresponding entry in the result set.
*/
/* static int processOrderGroupBy( */
/*   NameContext *pNC,     /1* Name context of the SELECT statement. *1/ */
/*   ExprList *pOrderBy,   /1* The ORDER BY or GROUP BY clause to be processed *1/ */
/*   const char *zType     /1* Either "ORDER" or "GROUP", as appropriate *1/ */
/* ){ */
/*   int i; */
/*   ExprList *pEList = pNC->pEList;     /1* The result set of the SELECT *1/ */
/*   Parse *pParse = pNC->pParse;     /1* The result set of the SELECT *1/ */
/*   assert( pEList ); */

/*   if( pOrderBy==0 ) return 0; */
/*   for(i=0; i<pOrderBy->nExpr; i++){ */
/*     int iCol; */
/*     Expr *pE = pOrderBy->a[i].pExpr; */
/*     if( sqlite3ExprIsInteger(pE, &iCol) ){ */
/*       if( iCol>0 && iCol<=pEList->nExpr ){ */
/*         sqlite3ExprDelete(pE); */
/*         pE = pOrderBy->a[i].pExpr = sqlite3ExprDup(pEList->a[iCol-1].pExpr); */
/*       }else{ */
/*         sqlite3ErrorMsg(pParse, */ 
/*            "%s BY column number %d out of range - should be " */
/*            "between 1 and %d", zType, iCol, pEList->nExpr); */
/*         return 1; */
/*       } */
/*     } */
/*     if( sqlite3ExprResolveNames(pNC, pE) ){ */
/*       return 1; */
/*     } */
/*     if( sqlite3ExprIsConstant(pE) ){ */
/*       sqlite3ErrorMsg(pParse, */
/*           "%s BY terms must not be non-integer constants", zType); */
/*       return 1; */
/*     } */
/*   } */
/*   return 0; */
/* } */

/*
** This routine resolves any names used in the result set of the
** supplied SELECT statement. If the SELECT statement being resolved
** is a sub-select, then pOuterNC is a pointer to the NameContext 
** of the parent SELECT.
*/
/* int sqlite3SelectResolve( */
/*   Parse *pParse,         /1* The parser context *1/ */
/*   Select *p,             /1* The SELECT statement being coded. *1/ */
/*   NameContext *pOuterNC  /1* The outer name context. May be NULL. *1/ */
/* ){ */
/*   ExprList *pEList;          /1* Result set. *1/ */
/*   int i;                     /1* For-loop variable used in multiple places *1/ */
/*   NameContext sNC;           /1* Local name-context *1/ */
/*   ExprList *pGroupBy;        /1* The group by clause *1/ */

/*   /1* If this routine has run before, return immediately. *1/ */
/*   if( p->isResolved ){ */
/*     assert( !pOuterNC ); */
/*     return SQLITE_OK; */
/*   } */
/*   p->isResolved = 1; */

/*   /1* If there have already been errors, do nothing. *1/ */
/*   if( pParse->nErr>0 ){ */
/*     return SQLITE_ERROR; */
/*   } */

/*   /1* Prepare the select statement. This call will allocate all cursors */
/*   ** required to handle the tables and subqueries in the FROM clause. */
/*   *1/ */
/*   if( prepSelectStmt(pParse, p) ){ */
/*     return SQLITE_ERROR; */
/*   } */

/*   /1* Resolve the expressions in the LIMIT and OFFSET clauses. These */
/*   ** are not allowed to refer to any names, so pass an empty NameContext. */
/*   *1/ */
/*   memset(&sNC, 0, sizeof(sNC)); */
/*   sNC.pParse = pParse; */
/*   if( sqlite3ExprResolveNames(&sNC, p->pLimit) || */
/*       sqlite3ExprResolveNames(&sNC, p->pOffset) ){ */
/*     return SQLITE_ERROR; */
/*   } */

/*   /1* Set up the local name-context to pass to ExprResolveNames() to */
/*   ** resolve the expression-list. */
/*   *1/ */
/*   sNC.allowAgg = 1; */
/*   sNC.pSrcList = p->pSrc; */
/*   sNC.pNext = pOuterNC; */

/*   /1* Resolve names in the result set. *1/ */
/*   pEList = p->pEList; */
/*   if( !pEList ) return SQLITE_ERROR; */
/*   for(i=0; i<pEList->nExpr; i++){ */
/*     Expr *pX = pEList->a[i].pExpr; */
/*     if( sqlite3ExprResolveNames(&sNC, pX) ){ */
/*       return SQLITE_ERROR; */
/*     } */
/*   } */

/*   /1* If there are no aggregate functions in the result-set, and no GROUP BY */ 
/*   ** expression, do not allow aggregates in any of the other expressions. */
/*   *1/ */
/*   assert( !p->isAgg ); */
/*   pGroupBy = p->pGroupBy; */
/*   if( pGroupBy || sNC.hasAgg ){ */
/*     p->isAgg = 1; */
/*   }else{ */
/*     sNC.allowAgg = 0; */
/*   } */

/*   /1* If a HAVING clause is present, then there must be a GROUP BY clause. */
/*   *1/ */
/*   if( p->pHaving && !pGroupBy ){ */
/*     sqlite3ErrorMsg(pParse, "a GROUP BY clause is required before HAVING"); */
/*     return SQLITE_ERROR; */
/*   } */

/*   /1* Add the expression list to the name-context before parsing the */
/*   ** other expressions in the SELECT statement. This is so that */
/*   ** expressions in the WHERE clause (etc.) can refer to expressions by */
/*   ** aliases in the result set. */
/*   ** */
/*   ** Minor point: If this is the case, then the expression will be */
/*   ** re-evaluated for each reference to it. */
/*   *1/ */
/*   sNC.pEList = p->pEList; */
/*   if( sqlite3ExprResolveNames(&sNC, p->pWhere) || */
/*       sqlite3ExprResolveNames(&sNC, p->pHaving) || */
/*       processOrderGroupBy(&sNC, p->pOrderBy, "ORDER") || */
/*       processOrderGroupBy(&sNC, pGroupBy, "GROUP") */
/*   ){ */
/*     return SQLITE_ERROR; */
/*   } */

/*   /1* Make sure the GROUP BY clause does not contain aggregate functions. */
/*   *1/ */
/*   if( pGroupBy ){ */
/*     struct ExprList_item *pItem; */
  
/*     for(i=0, pItem=pGroupBy->a; i<pGroupBy->nExpr; i++, pItem++){ */
/*       if( ExprHasProperty(pItem->pExpr, EP_Agg) ){ */
/*         sqlite3ErrorMsg(pParse, "aggregate functions are not allowed in " */
/*             "the GROUP BY clause"); */
/*         return SQLITE_ERROR; */
/*       } */
/*     } */
/*   } */

/*   return SQLITE_OK; */
/* } */

/*
** Reset the aggregate accumulator.
**
** The aggregate accumulator is a set of memory cells that hold
** intermediate results while calculating an aggregate.  This
** routine simply stores NULLs in all of those memory cells.
*/
/* static void resetAccumulator(Parse *pParse, AggInfo *pAggInfo){ */
/*   Vdbe *v = pParse->pVdbe; */
/*   int i; */
/*   struct AggInfo_func *pFunc; */
/*   if( pAggInfo->nFunc+pAggInfo->nColumn==0 ){ */
/*     return; */
/*   } */
/*   for(i=0; i<pAggInfo->nColumn; i++){ */
/*     sqlite3VdbeAddOp(v, OP_MemNull, pAggInfo->aCol[i].iMem, 0); */
/*   } */
/*   for(pFunc=pAggInfo->aFunc, i=0; i<pAggInfo->nFunc; i++, pFunc++){ */
/*     sqlite3VdbeAddOp(v, OP_MemNull, pFunc->iMem, 0); */
/*     if( pFunc->iDistinct>=0 ){ */
/*       Expr *pE = pFunc->pExpr; */
/*       if( pE->pList==0 || pE->pList->nExpr!=1 ){ */
/*         sqlite3ErrorMsg(pParse, "DISTINCT in aggregate must be followed " */
/*            "by an expression"); */
/*         pFunc->iDistinct = -1; */
/*       }else{ */
/*         KeyInfo *pKeyInfo = keyInfoFromExprList(pParse, pE->pList); */
/*         sqlite3VdbeOp3(v, OP_OpenVirtual, pFunc->iDistinct, 0, */ 
/*                           (char*)pKeyInfo, P3_KEYINFO_HANDOFF); */
/*       } */
/*     } */
/*   } */
/* } */

/*
** Invoke the OP_AggFinalize opcode for every aggregate function
** in the AggInfo structure.
*/
/* static void finalizeAggFunctions(Parse *pParse, AggInfo *pAggInfo){ */
/*   Vdbe *v = pParse->pVdbe; */
/*   int i; */
/*   struct AggInfo_func *pF; */
/*   for(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){ */
/*     ExprList *pList = pF->pExpr->pList; */
/*     sqlite3VdbeOp3(v, OP_AggFinal, pF->iMem, pList ? pList->nExpr : 0, */
/*                       (void*)pF->pFunc, P3_FUNCDEF); */
/*   } */
/* } */

/*
** Update the accumulator memory cells for an aggregate based on
** the current cursor position.
*/
/* static void updateAccumulator(Parse *pParse, AggInfo *pAggInfo){ */
/*   Vdbe *v = pParse->pVdbe; */
/*   int i; */
/*   struct AggInfo_func *pF; */
/*   struct AggInfo_col *pC; */

/*   pAggInfo->directMode = 1; */
/*   for(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){ */
/*     int nArg; */
/*     int addrNext = 0; */
/*     ExprList *pList = pF->pExpr->pList; */
/*     if( pList ){ */
/*       nArg = pList->nExpr; */
/*       sqlite3ExprCodeExprList(pParse, pList); */
/*     }else{ */
/*       nArg = 0; */
/*     } */
/*     if( pF->iDistinct>=0 ){ */
/*       addrNext = sqlite3VdbeMakeLabel(v); */
/*       assert( nArg==1 ); */
/*       codeDistinct(v, pF->iDistinct, addrNext, 1); */
/*     } */
/*     if( pF->pFunc->needCollSeq ){ */
/*       CollSeq *pColl = 0; */
/*       struct ExprList_item *pItem; */
/*       int j; */
/*       assert( pList!=0 );  /1* pList!=0 if pF->pFunc->needCollSeq is true *1/ */
/*       for(j=0, pItem=pList->a; !pColl && j<nArg; j++, pItem++){ */
/*         pColl = sqlite3ExprCollSeq(pParse, pItem->pExpr); */
/*       } */
/*       if( !pColl ){ */
/*         pColl = pParse->db->pDfltColl; */
/*       } */
/*       sqlite3VdbeOp3(v, OP_CollSeq, 0, 0, (char *)pColl, P3_COLLSEQ); */
/*     } */
/*     sqlite3VdbeOp3(v, OP_AggStep, pF->iMem, nArg, (void*)pF->pFunc, P3_FUNCDEF); */
/*     if( addrNext ){ */
/*       sqlite3VdbeResolveLabel(v, addrNext); */
/*     } */
/*   } */
/*   for(i=0, pC=pAggInfo->aCol; i<pAggInfo->nAccumulator; i++, pC++){ */
/*     sqlite3ExprCode(pParse, pC->pExpr); */
/*     sqlite3VdbeAddOp(v, OP_MemStore, pC->iMem, 1); */
/*   } */
/*   pAggInfo->directMode = 0; */
/* } */


/*
** Generate code for the given SELECT statement.
**
** The results are distributed in various ways depending on the
** value of eDest and iParm.
**
**     eDest Value       Result
**     ------------    -------------------------------------------
**     SRT_Callback    Invoke the callback for each row of the result.
**
**     SRT_Mem         Store first result in memory cell iParm
**
**     SRT_Set         Store results as keys of table iParm.
**
**     SRT_Union       Store results as a key in a temporary table iParm
**
**     SRT_Except      Remove results from the temporary table iParm.
**
**     SRT_Table       Store results in temporary table iParm
**
** The table above is incomplete.  Additional eDist value have be added
** since this comment was written.  See the selectInnerLoop() function for
** a complete listing of the allowed values of eDest and their meanings.
**
** This routine returns the number of errors.  If any errors are
** encountered, then an appropriate error message is left in
** pParse->zErrMsg.
**
** This routine does NOT free the Select structure passed in.  The
** calling function needs to do that.
**
** The pParent, parentTab, and *pParentAgg fields are filled in if this
** SELECT is a subquery.  This routine may try to combine this SELECT
** with its parent to form a single flat query.  In so doing, it might
** change the parent query from a non-aggregate to an aggregate query.
** For that reason, the pParentAgg flag is passed as a pointer, so it
** can be changed.
**
** Example 1:   The meaning of the pParent parameter.
**
**    SELECT * FROM t1 JOIN (SELECT x, count(*) FROM t2) JOIN t3;
**    \                      \_______ subquery _______/        /
**     \                                                      /
**      \____________________ outer query ___________________/
**
** This routine is called for the outer query first.   For that call,
** pParent will be NULL.  During the processing of the outer query, this 
** routine is called recursively to handle the subquery.  For the recursive
** call, pParent will point to the outer query.  Because the subquery is
** the second element in a three-way join, the parentTab parameter will
** be 1 (the 2nd value of a 0-indexed array.)
*/
int sqlite3Select(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  int eDest,             /* How to dispose of the results */
  int iParm,             /* A parameter used by the eDest disposal method */
  Select *pParent,       /* Another SELECT for which this is a sub-query */
  int parentTab,         /* Index in pParent->pSrc of this query */
  int *pParentAgg,       /* True if pParent uses aggregate functions */
  char *aff              /* If eDest is SRT_Union, the affinity string */
){
    int i, j;              /* Loop counters */
    WhereInfo *pWInfo;     /* Return from sqlite3WhereBegin() */
    int isAgg;             /* True for select lists like "count(*)" */
    ExprList *pEList;      /* List of columns to extract. */
    SrcList *pTabList;     /* List of tables to select from */
    Expr *pWhere;          /* The WHERE clause.  May be NULL */
    ExprList *pOrderBy;    /* The ORDER BY clause.  May be NULL */
    ExprList *pGroupBy;    /* The GROUP BY clause.  May be NULL */
    Expr *pHaving;         /* The HAVING clause.  May be NULL */
    int isDistinct;        /* True if the DISTINCT keyword is present */
    int distinct;          /* Table to use for the distinct set */
    int rc = 1;            /* Value to return from this function */
    int addrSortIndex;     /* Address of an OP_OpenVirtual instruction */
    AggInfo sAggInfo;      /* Information used by aggregate queries */
    int iEnd;              /* Address of the end of the query */

    if( p==0 || sqlite3MallocFailed() || pParse->nErr ){
        return 1;
    }
    memset(&sAggInfo, 0, sizeof(sAggInfo));

#ifndef SQLITE_OMIT_COMPOUND_SELECT
    /* If there is are a sequence of queries, do the earlier ones first.
    */
    if( p->pPrior ){
        if( p->pRightmost==0 ){
            Select *pLoop;
            for(pLoop=p; pLoop; pLoop=pLoop->pPrior){
                pLoop->pRightmost = p;
            }
        }
    }
#endif
    ParsedResultItem item;
    item.sqltype = SQLTYPE_SELECT;
    item.result.selectObj = p;
    sqlite3ParsedResultArrayAppend(&pParse->parsed, &item);
}
/********************************from the file update.c************************************/
/** This file contains C code routines that are called by the parser
** to handle UPDATE statements.
**
** $Id: update.c,v 1.123 2006/02/24 02:53:50 drh Exp $
*/

Update* sqlite3UpdateNew(SrcList *pTabList, ExprList *pChanges, Expr *pWhere, int onError, Expr *pLimit, Expr *pOffset) {
    Update *pNew = NULL;
    pNew = (Update*)sqliteMalloc( sizeof(*pNew) );
    if (pNew == NULL) {
        return NULL;
    }
    
    pNew->pTabList = pTabList;
    pNew->pChanges = pChanges;
    pNew->pWhere = pWhere;
    pNew->onError = onError;
    pNew->pLimit = pLimit;
    pNew->pOffset = pOffset;
    return pNew;
}

void sqlite3UpdateDelete(Update* updateObj) {
    if (updateObj == NULL) { return; }

    sqlite3SrcListDelete(updateObj->pTabList);
    sqlite3ExprListDelete(updateObj->pChanges);
    sqlite3ExprDelete(updateObj->pWhere);   
    sqlite3ExprDelete(updateObj->pLimit);
    sqlite3ExprDelete(updateObj->pOffset);
    sqliteFree(updateObj);
}

/*
** Process an UPDATE statement.
**
**   UPDATE OR IGNORE table_wxyz SET a=b, c=d WHERE e<5 AND f NOT NULL;
**          \_______/ \________/     \______/       \________________/
*            onError   pTabList      pChanges             pWhere
*/
void sqlite3Update(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table in which we should change things */
  ExprList *pChanges,    /* Things to be changed */
  Expr *pWhere,          /* The WHERE clause.  May be null */
  int onError,            /* How to handle constraint errors */
  Expr *pLimit,
  Expr *pOffset
){
    Update *updateObj = sqlite3UpdateNew(pTabList, pChanges, pWhere, onError, pLimit, pOffset);
    
    ParsedResultItem item;
    item.sqltype = SQLTYPE_UPDATE;
    item.result.updateObj = updateObj;
    sqlite3ParsedResultArrayAppend(&pParse->parsed, &item);
}
/***********************************file from util.c**********************************/
/** Utility functions used throughout sqlite.
**
** This file contains functions for allocating memory, comparing
** strings, and stuff like that.
**
** $Id: util.c,v 1.188 2006/04/04 01:54:55 drh Exp $
*/
#include <stdlib.h>
#include <ctype.h>
#define MAX(x,y) ((x)>(y)?(x):(y))

#if defined(SQLITE_ENABLE_MEMORY_MANAGEMENT) && !defined(SQLITE_OMIT_DISKIO)
/*
** Set the soft heap-size limit for the current thread. Passing a negative
** value indicates no limit.
*/
void sqlite3_soft_heap_limit(int n){
  ThreadData *pTd = sqlite3ThreadData();
  if( pTd ){
    pTd->nSoftHeapLimit = n;
  }
  sqlite3ReleaseThreadData();
}

/*
** Release memory held by SQLite instances created by the current thread.
*/
int sqlite3_release_memory(int n){
  return sqlite3pager_release_memory(n);
}
#else
/* If SQLITE_ENABLE_MEMORY_MANAGEMENT is not defined, then define a version
** of sqlite3_release_memory() to be used by other code in this file.
** This is done for no better reason than to reduce the number of 
** pre-processor #ifndef statements.
*/
#define sqlite3_release_memory(x) 0    /* 0 == no memory freed */
#endif

#ifdef SQLITE_MEMDEBUG
/*--------------------------------------------------------------------------
** Begin code for memory allocation system test layer.
**
** Memory debugging is turned on by defining the SQLITE_MEMDEBUG macro.
**
** SQLITE_MEMDEBUG==1    -> Fence-posting only (thread safe) 
** SQLITE_MEMDEBUG==2    -> Fence-posting + linked list of allocations (not ts)
** SQLITE_MEMDEBUG==3    -> Above + backtraces (not thread safe, req. glibc)
*/

/* Figure out whether or not to store backtrace() information for each malloc.
** The backtrace() function is only used if SQLITE_MEMDEBUG is set to 2 or 
** greater and glibc is in use. If we don't want to use backtrace(), then just
** define it as an empty macro and set the amount of space reserved to 0.
*/
#if defined(__GLIBC__) && SQLITE_MEMDEBUG>2
  extern int backtrace(void **, int);
  #define TESTALLOC_STACKSIZE 128
  #define TESTALLOC_STACKFRAMES ((TESTALLOC_STACKSIZE-8)/sizeof(void*))
#else
  #define backtrace(x, y)
  #define TESTALLOC_STACKSIZE 0
  #define TESTALLOC_STACKFRAMES 0
#endif

/*
** Number of 32-bit guard words.  This should probably be a multiple of
** 2 since on 64-bit machines we want the value returned by sqliteMalloc()
** to be 8-byte aligned.
*/
#ifndef TESTALLOC_NGUARD
# define TESTALLOC_NGUARD 2
#endif

/*
** Size reserved for storing file-name along with each malloc()ed blob.
*/
#define TESTALLOC_FILESIZE 64

/*
** Size reserved for storing the user string. Each time a Malloc() or Realloc()
** call succeeds, up to TESTALLOC_USERSIZE bytes of the string pointed to by
** sqlite3_malloc_id are stored along with the other test system metadata.
*/
#define TESTALLOC_USERSIZE 64
const char *sqlite3_malloc_id = 0;

/*
** Blocks used by the test layer have the following format:
**
**        <sizeof(void *) pNext pointer>
**        <sizeof(void *) pPrev pointer>
**        <TESTALLOC_NGUARD 32-bit guard words>
**            <The application level allocation>
**        <TESTALLOC_NGUARD 32-bit guard words>
**        <32-bit line number>
**        <TESTALLOC_FILESIZE bytes containing null-terminated file name>
**        <TESTALLOC_STACKSIZE bytes of backtrace() output>
*/ 

#define TESTALLOC_OFFSET_GUARD1(p)    (sizeof(void *) * 2)
#define TESTALLOC_OFFSET_DATA(p) ( \
  TESTALLOC_OFFSET_GUARD1(p) + sizeof(u32) * TESTALLOC_NGUARD \
)
#define TESTALLOC_OFFSET_GUARD2(p) ( \
  TESTALLOC_OFFSET_DATA(p) + sqlite3OsAllocationSize(p) - TESTALLOC_OVERHEAD \
)
#define TESTALLOC_OFFSET_LINENUMBER(p) ( \
  TESTALLOC_OFFSET_GUARD2(p) + sizeof(u32) * TESTALLOC_NGUARD \
)
#define TESTALLOC_OFFSET_FILENAME(p) ( \
  TESTALLOC_OFFSET_LINENUMBER(p) + sizeof(u32) \
)
#define TESTALLOC_OFFSET_USER(p) ( \
  TESTALLOC_OFFSET_FILENAME(p) + TESTALLOC_FILESIZE \
)
#define TESTALLOC_OFFSET_STACK(p) ( \
  TESTALLOC_OFFSET_USER(p) + TESTALLOC_USERSIZE + 8 - \
  (TESTALLOC_OFFSET_USER(p) % 8) \
)

#define TESTALLOC_OVERHEAD ( \
  sizeof(void *)*2 +                   /* pPrev and pNext pointers */   \
  TESTALLOC_NGUARD*sizeof(u32)*2 +              /* Guard words */       \
  sizeof(u32) + TESTALLOC_FILESIZE +   /* File and line number */       \
  TESTALLOC_USERSIZE +                 /* User string */                \
  TESTALLOC_STACKSIZE                  /* backtrace() stack */          \
)


/*
** For keeping track of the number of mallocs and frees.   This
** is used to check for memory leaks.  The iMallocFail and iMallocReset
** values are used to simulate malloc() failures during testing in 
** order to verify that the library correctly handles an out-of-memory
** condition.
*/
int sqlite3_nMalloc;         /* Number of sqliteMalloc() calls */
int sqlite3_nFree;           /* Number of sqliteFree() calls */
int sqlite3_memUsed;         /* TODO Total memory obtained from malloc */
int sqlite3_memMax;          /* TODO Mem usage high-water mark */
int sqlite3_iMallocFail;     /* Fail sqliteMalloc() after this many calls */
int sqlite3_iMallocReset = -1; /* When iMallocFail reaches 0, set to this */

void *sqlite3_pFirst = 0;         /* Pointer to linked list of allocations */
int sqlite3_nMaxAlloc = 0;        /* High water mark of ThreadData.nAlloc */
int sqlite3_mallocDisallowed = 0; /* assert() in sqlite3Malloc() if set */
int sqlite3_isFail = 0;           /* True if all malloc calls should fail */
const char *sqlite3_zFile = 0;    /* Filename to associate debug info with */
int sqlite3_iLine = 0;            /* Line number for debug info */

/*
** Check for a simulated memory allocation failure.  Return true if
** the failure should be simulated.  Return false to proceed as normal.
*/
int sqlite3TestMallocFail(){
  if( sqlite3_isFail ){
    return 1;
  }
  if( sqlite3_iMallocFail>=0 ){
    sqlite3_iMallocFail--;
    if( sqlite3_iMallocFail==0 ){
      sqlite3_iMallocFail = sqlite3_iMallocReset;
      sqlite3_isFail = 1;
      return 1;
    }
  }
  return 0;
}

/*
** The argument is a pointer returned by sqlite3OsMalloc() or xRealloc().
** assert() that the first and last (TESTALLOC_NGUARD*4) bytes are set to the
** values set by the applyGuards() function.
*/
static void checkGuards(u32 *p)
{
  int i;
  char *zAlloc = (char *)p;
  char *z;

  /* First set of guard words */
  z = &zAlloc[TESTALLOC_OFFSET_GUARD1(p)];
  for(i=0; i<TESTALLOC_NGUARD; i++){
    assert(((u32 *)z)[i]==0xdead1122);
  }

  /* Second set of guard words */
  z = &zAlloc[TESTALLOC_OFFSET_GUARD2(p)];
  for(i=0; i<TESTALLOC_NGUARD; i++){
    u32 guard = 0;
    memcpy(&guard, &z[i*sizeof(u32)], sizeof(u32));
    assert(guard==0xdead3344);
  }
}

/*
** The argument is a pointer returned by sqlite3OsMalloc() or Realloc(). The
** first and last (TESTALLOC_NGUARD*4) bytes are set to known values for use as 
** guard-posts.
*/
static void applyGuards(u32 *p)
{
  int i;
  char *z;
  char *zAlloc = (char *)p;

  /* First set of guard words */
  z = &zAlloc[TESTALLOC_OFFSET_GUARD1(p)];
  for(i=0; i<TESTALLOC_NGUARD; i++){
    ((u32 *)z)[i] = 0xdead1122;
  }

  /* Second set of guard words */
  z = &zAlloc[TESTALLOC_OFFSET_GUARD2(p)];
  for(i=0; i<TESTALLOC_NGUARD; i++){
    static const int guard = 0xdead3344;
    memcpy(&z[i*sizeof(u32)], &guard, sizeof(u32));
  }

  /* Line number */
  z = &((char *)z)[TESTALLOC_NGUARD*sizeof(u32)];             /* Guard words */
  z = &zAlloc[TESTALLOC_OFFSET_LINENUMBER(p)];
  memcpy(z, &sqlite3_iLine, sizeof(u32));

  /* File name */
  z = &zAlloc[TESTALLOC_OFFSET_FILENAME(p)];
  strncpy(z, sqlite3_zFile, TESTALLOC_FILESIZE);
  z[TESTALLOC_FILESIZE - 1] = '\0';

  /* User string */
  z = &zAlloc[TESTALLOC_OFFSET_USER(p)];
  z[0] = 0;
  if( sqlite3_malloc_id ){
    strncpy(z, sqlite3_malloc_id, TESTALLOC_USERSIZE);
    z[TESTALLOC_USERSIZE-1] = 0;
  }

  /* backtrace() stack */
  z = &zAlloc[TESTALLOC_OFFSET_STACK(p)];
  backtrace((void **)z, TESTALLOC_STACKFRAMES);

  /* Sanity check to make sure checkGuards() is working */
  checkGuards(p);
}

/*
** The argument is a malloc()ed pointer as returned by the test-wrapper.
** Return a pointer to the Os level allocation.
*/
static void *getOsPointer(void *p)
{
  char *z = (char *)p;
  return (void *)(&z[-1 * TESTALLOC_OFFSET_DATA(p)]);
}


#if SQLITE_MEMDEBUG>1
/*
** The argument points to an Os level allocation. Link it into the threads list
** of allocations.
*/
static void linkAlloc(void *p){
  void **pp = (void **)p;
  pp[0] = 0;
  pp[1] = sqlite3_pFirst;
  if( sqlite3_pFirst ){
    ((void **)sqlite3_pFirst)[0] = p;
  }
  sqlite3_pFirst = p;
}

/*
** The argument points to an Os level allocation. Unlinke it from the threads
** list of allocations.
*/
static void unlinkAlloc(void *p)
{
  void **pp = (void **)p;
  if( p==sqlite3_pFirst ){
    assert(!pp[0]);
    assert(!pp[1] || ((void **)(pp[1]))[0]==p);
    sqlite3_pFirst = pp[1];
    if( sqlite3_pFirst ){
      ((void **)sqlite3_pFirst)[0] = 0;
    }
  }else{
    void **pprev = pp[0];
    void **pnext = pp[1];
    assert(pprev);
    assert(pprev[1]==p);
    pprev[1] = (void *)pnext;
    if( pnext ){
      assert(pnext[0]==p);
      pnext[0] = (void *)pprev;
    }
  }
}

/*
** Pointer p is a pointer to an OS level allocation that has just been
** realloc()ed. Set the list pointers that point to this entry to it's new
** location.
*/
static void relinkAlloc(void *p)
{
  void **pp = (void **)p;
  if( pp[0] ){
    ((void **)(pp[0]))[1] = p;
  }else{
    sqlite3_pFirst = p;
  }
  if( pp[1] ){
    ((void **)(pp[1]))[0] = p;
  }
}
#else
#define linkAlloc(x)
#define relinkAlloc(x)
#define unlinkAlloc(x)
#endif

/*
** This function sets the result of the Tcl interpreter passed as an argument
** to a list containing an entry for each currently outstanding call made to 
** sqliteMalloc and friends by the current thread. Each list entry is itself a
** list, consisting of the following (in order):
**
**     * The number of bytes allocated
**     * The __FILE__ macro at the time of the sqliteMalloc() call.
**     * The __LINE__ macro ...
**     * The value of the sqlite3_malloc_id variable ...
**     * The output of backtrace() (if available) ...
**
** Todo: We could have a version of this function that outputs to stdout, 
** to debug memory leaks when Tcl is not available.
*/
#if defined(TCLSH) && defined(SQLITE_DEBUG) && SQLITE_MEMDEBUG>1
#include <tcl.h>
int sqlite3OutstandingMallocs(Tcl_Interp *interp){
  void *p;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_IncrRefCount(pRes);


  for(p=sqlite3_pFirst; p; p=((void **)p)[1]){
    Tcl_Obj *pEntry = Tcl_NewObj();
    Tcl_Obj *pStack = Tcl_NewObj();
    char *z;
    u32 iLine;
    int nBytes = sqlite3OsAllocationSize(p) - TESTALLOC_OVERHEAD;
    char *zAlloc = (char *)p;
    int i;

    Tcl_ListObjAppendElement(0, pEntry, Tcl_NewIntObj(nBytes));

    z = &zAlloc[TESTALLOC_OFFSET_FILENAME(p)];
    Tcl_ListObjAppendElement(0, pEntry, Tcl_NewStringObj(z, -1));

    z = &zAlloc[TESTALLOC_OFFSET_LINENUMBER(p)];
    memcpy(&iLine, z, sizeof(u32));
    Tcl_ListObjAppendElement(0, pEntry, Tcl_NewIntObj(iLine));

    z = &zAlloc[TESTALLOC_OFFSET_USER(p)];
    Tcl_ListObjAppendElement(0, pEntry, Tcl_NewStringObj(z, -1));

    z = &zAlloc[TESTALLOC_OFFSET_STACK(p)];
    for(i=0; i<TESTALLOC_STACKFRAMES; i++){
      char zHex[128];
      sprintf(zHex, "%p", ((void **)z)[i]);
      Tcl_ListObjAppendElement(0, pStack, Tcl_NewStringObj(zHex, -1));
    }

    Tcl_ListObjAppendElement(0, pEntry, pStack);
    Tcl_ListObjAppendElement(0, pRes, pEntry);
  }

  Tcl_ResetResult(interp);
  Tcl_SetObjResult(interp, pRes);
  Tcl_DecrRefCount(pRes);
  return TCL_OK;
}
#endif

/*
** This is the test layer's wrapper around sqlite3OsMalloc().
*/
static void * OSMALLOC(int n){
  sqlite3OsEnterMutex();
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  sqlite3_nMaxAlloc = 
      MAX(sqlite3_nMaxAlloc, sqlite3ThreadDataReadOnly()->nAlloc);
#endif
  assert( !sqlite3_mallocDisallowed );
  if( !sqlite3TestMallocFail() ){
    u32 *p;
    p = (u32 *)sqlite3OsMalloc(n + TESTALLOC_OVERHEAD);
    assert(p);
    sqlite3_nMalloc++;
    applyGuards(p);
    linkAlloc(p);
    sqlite3OsLeaveMutex();
    return (void *)(&p[TESTALLOC_NGUARD + 2*sizeof(void *)/sizeof(u32)]);
  }
  sqlite3OsLeaveMutex();
  return 0;
}

static int OSSIZEOF(void *p){
  if( p ){
    u32 *pOs = (u32 *)getOsPointer(p);
    return sqlite3OsAllocationSize(pOs) - TESTALLOC_OVERHEAD;
  }
  return 0;
}

/*
** This is the test layer's wrapper around sqlite3OsFree(). The argument is a
** pointer to the space allocated for the application to use.
*/
static void OSFREE(void *pFree){
  sqlite3OsEnterMutex();
  u32 *p = (u32 *)getOsPointer(pFree);   /* p points to Os level allocation */
  checkGuards(p);
  unlinkAlloc(p);
  memset(pFree, 0x55, OSSIZEOF(pFree));
  sqlite3OsFree(p);
  sqlite3_nFree++;
  sqlite3OsLeaveMutex();
}

/*
** This is the test layer's wrapper around sqlite3OsRealloc().
*/
static void * OSREALLOC(void *pRealloc, int n){
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  sqlite3_nMaxAlloc = 
      MAX(sqlite3_nMaxAlloc, sqlite3ThreadDataReadOnly()->nAlloc);
#endif
  assert( !sqlite3_mallocDisallowed );
  if( !sqlite3TestMallocFail() ){
    u32 *p = (u32 *)getOsPointer(pRealloc);
    checkGuards(p);
    p = sqlite3OsRealloc(p, n + TESTALLOC_OVERHEAD);
    applyGuards(p);
    relinkAlloc(p);
    return (void *)(&p[TESTALLOC_NGUARD + 2*sizeof(void *)/sizeof(u32)]);
  }
  return 0;
}

static void OSMALLOC_FAILED(){
  sqlite3_isFail = 0;
}

#else
/* Define macros to call the sqlite3OsXXX interface directly if 
** the SQLITE_MEMDEBUG macro is not defined.
*/
#define OSMALLOC(x)        sqlite3OsMalloc(x)
#define OSREALLOC(x,y)     sqlite3OsRealloc(x,y)
#define OSFREE(x)          sqlite3OsFree(x)
#define OSSIZEOF(x)        sqlite3OsAllocationSize(x)
#define OSMALLOC_FAILED()

#endif  /* SQLITE_MEMDEBUG */
/*
** End code for memory allocation system test layer.
**--------------------------------------------------------------------------*/

/*
** This routine is called when we are about to allocate n additional bytes
** of memory.  If the new allocation will put is over the soft allocation
** limit, then invoke sqlite3_release_memory() to try to release some
** memory before continuing with the allocation.
**
** This routine also makes sure that the thread-specific-data (TSD) has
** be allocated.  If it has not and can not be allocated, then return
** false.  The updateMemoryUsedCount() routine below will deallocate
** the TSD if it ought to be.
**
** If SQLITE_ENABLE_MEMORY_MANAGEMENT is not defined, this routine is
** a no-op
*/ 
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
static int enforceSoftLimit(int n){
  ThreadData *pTsd = sqlite3ThreadData();
  if( pTsd==0 ){
    return 0;
  }
  assert( pTsd->nAlloc>=0 );
  if( n>0 && pTsd->nSoftHeapLimit>0 ){
    while( pTsd->nAlloc+n>pTsd->nSoftHeapLimit && sqlite3_release_memory(n) ){}
  }
  return 1;
}
#else
# define enforceSoftLimit(X)  1
#endif

/*
** Update the count of total outstanding memory that is held in
** thread-specific-data (TSD).  If after this update the TSD is
** no longer being used, then deallocate it.
**
** If SQLITE_ENABLE_MEMORY_MANAGEMENT is not defined, this routine is
** a no-op
*/
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
static void updateMemoryUsedCount(int n){
  ThreadData *pTsd = sqlite3ThreadData();
  if( pTsd ){
    pTsd->nAlloc += n;
    assert( pTsd->nAlloc>=0 );
    if( pTsd->nAlloc==0 && pTsd->nSoftHeapLimit==0 ){
      sqlite3ReleaseThreadData();
    }
  }
}
#else
#define updateMemoryUsedCount(x)  /* no-op */
#endif

/*
** Allocate and return N bytes of uninitialised memory by calling
** sqlite3OsMalloc(). If the Malloc() call fails, attempt to free memory 
** by calling sqlite3_release_memory().
*/
void *sqlite3MallocRaw(int n, int doMemManage){
  void *p = 0;
  if( n>0 && !sqlite3MallocFailed() && (!doMemManage || enforceSoftLimit(n)) ){
    while( (p = OSMALLOC(n))==0 && sqlite3_release_memory(n) ){}
    if( !p ){
      sqlite3FailedMalloc();
      OSMALLOC_FAILED();
    }else if( doMemManage ){
      updateMemoryUsedCount(OSSIZEOF(p));
    }
  }
  return p;
}

/*
** Resize the allocation at p to n bytes by calling sqlite3OsRealloc(). The
** pointer to the new allocation is returned.  If the Realloc() call fails,
** attempt to free memory by calling sqlite3_release_memory().
*/
void *sqlite3Realloc(void *p, int n){
  if( sqlite3MallocFailed() ){
    return 0;
  }

  if( !p ){
    return sqlite3Malloc(n, 1);
  }else{
    void *np = 0;
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    int origSize = OSSIZEOF(p);
#endif
    if( enforceSoftLimit(n - origSize) ){
      while( (np = OSREALLOC(p, n))==0 && sqlite3_release_memory(n) ){}
      if( !np ){
        sqlite3FailedMalloc();
        OSMALLOC_FAILED();
      }else{
        updateMemoryUsedCount(OSSIZEOF(np) - origSize);
      }
    }
    return np;
  }
}

/*
** Free the memory pointed to by p. p must be either a NULL pointer or a 
** value returned by a previous call to sqlite3Malloc() or sqlite3Realloc().
*/
void sqlite3FreeX(void *p){
  if( p ){
    updateMemoryUsedCount(0 - OSSIZEOF(p));
    OSFREE(p);
  }
}

/*
** A version of sqliteMalloc() that is always a function, not a macro.
** Currently, this is used only to alloc to allocate the parser engine.
*/
void *sqlite3MallocX(int n){
  return sqliteMalloc(n);
}

/*
** sqlite3Malloc
** sqlite3ReallocOrFree
**
** These two are implemented as wrappers around sqlite3MallocRaw(), 
** sqlite3Realloc() and sqlite3Free().
*/ 
void *sqlite3Malloc(int n, int doMemManage){
  void *p = sqlite3MallocRaw(n, doMemManage);
  if( p ){
    memset(p, 0, n);
  }
  return p;
}
void sqlite3ReallocOrFree(void **pp, int n){
  void *p = sqlite3Realloc(*pp, n);
  if( !p ){
    sqlite3FreeX(*pp);
  }
  *pp = p;
}

/*
** sqlite3ThreadSafeMalloc() and sqlite3ThreadSafeFree() are used in those
** rare scenarios where sqlite may allocate memory in one thread and free
** it in another. They are exactly the same as sqlite3Malloc() and 
** sqlite3Free() except that:
**
**   * The allocated memory is not included in any calculations with 
**     respect to the soft-heap-limit, and
**
**   * sqlite3ThreadSafeMalloc() must be matched with ThreadSafeFree(),
**     not sqlite3Free(). Calling sqlite3Free() on memory obtained from
**     ThreadSafeMalloc() will cause an error somewhere down the line.
*/
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
void *sqlite3ThreadSafeMalloc(int n){
  ENTER_MALLOC;
  return sqlite3Malloc(n, 0);
}
void sqlite3ThreadSafeFree(void *p){
  ENTER_MALLOC;
  if( p ){
    OSFREE(p);
  }
}
#endif


/*
** Return the number of bytes allocated at location p. p must be either 
** a NULL pointer (in which case 0 is returned) or a pointer returned by 
** sqlite3Malloc(), sqlite3Realloc() or sqlite3ReallocOrFree().
**
** The number of bytes allocated does not include any overhead inserted by 
** any malloc() wrapper functions that may be called. So the value returned
** is the number of bytes that were available to SQLite using pointer p, 
** regardless of how much memory was actually allocated.
*/
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
int sqlite3AllocSize(void *p){
  return OSSIZEOF(p);
}
#endif

/*
** Make a copy of a string in memory obtained from sqliteMalloc(). These 
** functions call sqlite3MallocRaw() directly instead of sqliteMalloc(). This
** is because when memory debugging is turned on, these two functions are 
** called via macros that record the current file and line number in the
** ThreadData structure.
*/
char *sqlite3StrDup(const char *z){
  char *zNew;
  if( z==0 ) return 0;
  zNew = sqlite3MallocRaw(strlen(z)+1, 1);
  if( zNew ) strcpy(zNew, z);
  return zNew;
}
char *sqlite3StrNDup(const char *z, int n){
  char *zNew;
  if( z==0 ) return 0;
  zNew = sqlite3MallocRaw(n+1, 1);
  if( zNew ){
    memcpy(zNew, z, n);
    zNew[n] = 0;
  }
  return zNew;
}

/*
** Create a string from the 2nd and subsequent arguments (up to the
** first NULL argument), store the string in memory obtained from
** sqliteMalloc() and make the pointer indicated by the 1st argument
** point to that string.  The 1st argument must either be NULL or 
** point to memory obtained from sqliteMalloc().
*/
void sqlite3SetString(char **pz, ...){
  va_list ap;
  int nByte;
  const char *z;
  char *zResult;

  if( pz==0 ) return;
  nByte = 1;
  va_start(ap, pz);
  while( (z = va_arg(ap, const char*))!=0 ){
    nByte += strlen(z);
  }
  va_end(ap);
  sqliteFree(*pz);
  *pz = zResult = sqliteMallocRaw( nByte );
  if( zResult==0 ){
    return;
  }
  *zResult = 0;
  va_start(ap, pz);
  while( (z = va_arg(ap, const char*))!=0 ){
    strcpy(zResult, z);
    zResult += strlen(zResult);
  }
  va_end(ap);
}

/*
** Set the most recent error code and error string for the sqlite
** handle "db". The error code is set to "err_code".
**
** If it is not NULL, string zFormat specifies the format of the
** error string in the style of the printf functions: The following
** format characters are allowed:
**
**      %s      Insert a string
**      %z      A string that should be freed after use
**      %d      Insert an integer
**      %T      Insert a token
**      %S      Insert the first element of a SrcList
**
** zFormat and any string tokens that follow it are assumed to be
** encoded in UTF-8.
**
** To clear the most recent error for sqlite handle "db", sqlite3Error
** should be called with err_code set to SQLITE_OK and zFormat set
** to NULL.
*/
/* void sqlite3Error(sqlite3 *db, int err_code, const char *zFormat, ...){ */
/*   if( db && (db->pErr || (db->pErr = sqlite3ValueNew())!=0) ){ */
/*     db->errCode = err_code; */
/*     if( zFormat ){ */
/*       char *z; */
/*       va_list ap; */
/*       va_start(ap, zFormat); */
/*       z = sqlite3VMPrintf(zFormat, ap); */
/*       va_end(ap); */
/*       sqlite3ValueSetStr(db->pErr, -1, z, SQLITE_UTF8, sqlite3FreeX); */
/*     }else{ */
/*       sqlite3ValueSetStr(db->pErr, 0, 0, SQLITE_UTF8, SQLITE_STATIC); */
/*     } */
/*   } */
/* } */

/*
** Add an error message to pParse->zErrMsg and increment pParse->nErr.
** The following formatting characters are allowed:
**
**      %s      Insert a string
**      %z      A string that should be freed after use
**      %d      Insert an integer
**      %T      Insert a token
**      %S      Insert the first element of a SrcList
**
** This function should be used to report any error that occurs whilst
** compiling an SQL statement (i.e. within sqlite3_prepare()). The
** last thing the sqlite3_prepare() function does is copy the error
** stored by this function into the database handle using sqlite3Error().
** Function sqlite3Error() should be used during statement execution
** (sqlite3_step() etc.).
*/
void sqlite3ErrorMsg(Parse *pParse, const char *zFormat, ...){
  va_list ap;
  pParse->nErr++;
  sqliteFree(pParse->zErrMsg);
  va_start(ap, zFormat);
  pParse->zErrMsg = sqlite3VMPrintf(zFormat, ap);
  va_end(ap);
}

/*
** Clear the error message in pParse, if any
*/
void sqlite3ErrorClear(Parse *pParse){
  sqliteFree(pParse->zErrMsg);
  pParse->zErrMsg = 0;
  pParse->nErr = 0;
}

/*
** Convert an SQL-style quoted string into a normal string by removing
** the quote characters.  The conversion is done in-place.  If the
** input does not begin with a quote character, then this routine
** is a no-op.
**
** 2002-Feb-14: This routine is extended to remove MS-Access style
** brackets from around identifers.  For example:  "[a-b-c]" becomes
** "a-b-c".
*/
void sqlite3Dequote(char *z){
  int quote;
  int i, j;
  if( z==0 ) return;
  quote = z[0];
  switch( quote ){
    case '\'':  break;
    case '"':   break;
    case '`':   break;                /* For MySQL compatibility */
    case '[':   quote = ']';  break;  /* For MS SqlServer compatibility */
    default:    return;
  }
  for(i=1, j=0; z[i]; i++){
    if( z[i]==quote ){
      if( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }else{
        z[j++] = 0;
        break;
      }
    }else{
      z[j++] = z[i];
    }
  }
}

/* An array to map all upper-case characters into their corresponding
** lower-case character. 
*/
const unsigned char sqlite3UpperToLower[] = {
#ifdef SQLITE_ASCII
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
     18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
     36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
     54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 97, 98, 99,100,101,102,103,
    104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,
    122, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,101,102,103,104,105,106,107,
    108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,
    126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,
    162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,
    180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,
    198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,
    216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,
    234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,
    252,253,254,255
#endif
#ifdef SQLITE_EBCDIC
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, /* 0x */
     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, /* 1x */
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, /* 2x */
     48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, /* 3x */
     64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, /* 4x */
     80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, /* 5x */
     96, 97, 66, 67, 68, 69, 70, 71, 72, 73,106,107,108,109,110,111, /* 6x */
    112, 81, 82, 83, 84, 85, 86, 87, 88, 89,122,123,124,125,126,127, /* 7x */
    128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143, /* 8x */
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,156,159, /* 9x */
    160,161,162,163,164,165,166,167,168,169,170,171,140,141,142,175, /* Ax */
    176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191, /* Bx */
    192,129,130,131,132,133,134,135,136,137,202,203,204,205,206,207, /* Cx */
    208,145,146,147,148,149,150,151,152,153,218,219,220,221,222,223, /* Dx */
    224,225,162,163,164,165,166,167,168,169,232,203,204,205,206,207, /* Ex */
    239,240,241,242,243,244,245,246,247,248,249,219,220,221,222,255, /* Fx */
#endif
};
#define UpperToLower sqlite3UpperToLower

/*
** Some systems have stricmp().  Others have strcasecmp().  Because
** there is no consistency, we will define our own.
*/
int sqlite3StrICmp(const char *zLeft, const char *zRight){
  register unsigned char *a, *b;
  a = (unsigned char *)zLeft;
  b = (unsigned char *)zRight;
  while( *a!=0 && UpperToLower[*a]==UpperToLower[*b]){ a++; b++; }
  return UpperToLower[*a] - UpperToLower[*b];
}
int sqlite3StrNICmp(const char *zLeft, const char *zRight, int N){
  register unsigned char *a, *b;
  a = (unsigned char *)zLeft;
  b = (unsigned char *)zRight;
  while( N-- > 0 && *a!=0 && UpperToLower[*a]==UpperToLower[*b]){ a++; b++; }
  return N<0 ? 0 : UpperToLower[*a] - UpperToLower[*b];
}

/*
** Return TRUE if z is a pure numeric string.  Return FALSE if the
** string contains any character which is not part of a number. If
** the string is numeric and contains the '.' character, set *realnum
** to TRUE (otherwise FALSE).
**
** An empty string is considered non-numeric.
*/
int sqlite3IsNumber(const char *z, int *realnum, u8 enc){
  int incr = (enc==SQLITE_UTF8?1:2);
  if( enc==SQLITE_UTF16BE ) z++;
  if( *z=='-' || *z=='+' ) z += incr;
  if( !isdigit(*(u8*)z) ){
    return 0;
  }
  z += incr;
  if( realnum ) *realnum = 0;
  while( isdigit(*(u8*)z) ){ z += incr; }
  if( *z=='.' ){
    z += incr;
    if( !isdigit(*(u8*)z) ) return 0;
    while( isdigit(*(u8*)z) ){ z += incr; }
    if( realnum ) *realnum = 1;
  }
  if( *z=='e' || *z=='E' ){
    z += incr;
    if( *z=='+' || *z=='-' ) z += incr;
    if( !isdigit(*(u8*)z) ) return 0;
    while( isdigit(*(u8*)z) ){ z += incr; }
    if( realnum ) *realnum = 1;
  }
  return *z==0;
}

/*
** The string z[] is an ascii representation of a real number.
** Convert this string to a double.
**
** This routine assumes that z[] really is a valid number.  If it
** is not, the result is undefined.
**
** This routine is used instead of the library atof() function because
** the library atof() might want to use "," as the decimal point instead
** of "." depending on how locale is set.  But that would cause problems
** for SQL.  So this routine always uses "." regardless of locale.
*/
int sqlite3AtoF(const char *z, double *pResult){
#ifndef SQLITE_OMIT_FLOATING_POINT
  int sign = 1;
  const char *zBegin = z;
  LONGDOUBLE_TYPE v1 = 0.0;
  while( isspace(*z) ) z++;
  if( *z=='-' ){
    sign = -1;
    z++;
  }else if( *z=='+' ){
    z++;
  }
  while( isdigit(*(u8*)z) ){
    v1 = v1*10.0 + (*z - '0');
    z++;
  }
  if( *z=='.' ){
    LONGDOUBLE_TYPE divisor = 1.0;
    z++;
    while( isdigit(*(u8*)z) ){
      v1 = v1*10.0 + (*z - '0');
      divisor *= 10.0;
      z++;
    }
    v1 /= divisor;
  }
  if( *z=='e' || *z=='E' ){
    int esign = 1;
    int eval = 0;
    LONGDOUBLE_TYPE scale = 1.0;
    z++;
    if( *z=='-' ){
      esign = -1;
      z++;
    }else if( *z=='+' ){
      z++;
    }
    while( isdigit(*(u8*)z) ){
      eval = eval*10 + *z - '0';
      z++;
    }
    while( eval>=64 ){ scale *= 1.0e+64; eval -= 64; }
    while( eval>=16 ){ scale *= 1.0e+16; eval -= 16; }
    while( eval>=4 ){ scale *= 1.0e+4; eval -= 4; }
    while( eval>=1 ){ scale *= 1.0e+1; eval -= 1; }
    if( esign<0 ){
      v1 /= scale;
    }else{
      v1 *= scale;
    }
  }
  *pResult = sign<0 ? -v1 : v1;
  return z - zBegin;
#else
  return sqlite3atoi64(z, pResult);
#endif /* SQLITE_OMIT_FLOATING_POINT */
}

/*
** Return TRUE if zNum is a 64-bit signed integer and write
** the value of the integer into *pNum.  If zNum is not an integer
** or is an integer that is too large to be expressed with 64 bits,
** then return false.  If n>0 and the integer is string is not
** exactly n bytes long, return false.
**
** When this routine was originally written it dealt with only
** 32-bit numbers.  At that time, it was much faster than the
** atoi() library routine in RedHat 7.2.
*/
int sqlite3atoi64(const char *zNum, i64 *pNum){
  i64 v = 0;
  int neg;
  int i, c;
  while( isspace(*zNum) ) zNum++;
  if( *zNum=='-' ){
    neg = 1;
    zNum++;
  }else if( *zNum=='+' ){
    neg = 0;
    zNum++;
  }else{
    neg = 0;
  }
  for(i=0; (c=zNum[i])>='0' && c<='9'; i++){
    v = v*10 + c - '0';
  }
  *pNum = neg ? -v : v;
  return c==0 && i>0 && 
      (i<19 || (i==19 && memcmp(zNum,"9223372036854775807",19)<=0));
}

/*
** The string zNum represents an integer.  There might be some other
** information following the integer too, but that part is ignored.
** If the integer that the prefix of zNum represents will fit in a
** 32-bit signed integer, return TRUE.  Otherwise return FALSE.
**
** This routine returns FALSE for the string -2147483648 even that
** that number will in fact fit in a 32-bit integer.  But positive
** 2147483648 will not fit in 32 bits.  So it seems safer to return
** false.
*/
static int sqlite3FitsIn32Bits(const char *zNum){
  int i, c;
  if( *zNum=='-' || *zNum=='+' ) zNum++;
  for(i=0; (c=zNum[i])>='0' && c<='9'; i++){}
  return i<10 || (i==10 && memcmp(zNum,"2147483647",10)<=0);
}

/*
** If zNum represents an integer that will fit in 32-bits, then set
** *pValue to that integer and return true.  Otherwise return false.
*/
int sqlite3GetInt32(const char *zNum, int *pValue){
  if( sqlite3FitsIn32Bits(zNum) ){
    *pValue = atoi(zNum);
    return 1;
  }
  return 0;
}

/*
** The string zNum represents an integer.  There might be some other
** information following the integer too, but that part is ignored.
** If the integer that the prefix of zNum represents will fit in a
** 64-bit signed integer, return TRUE.  Otherwise return FALSE.
**
** This routine returns FALSE for the string -9223372036854775808 even that
** that number will, in theory fit in a 64-bit integer.  Positive
** 9223373036854775808 will not fit in 64 bits.  So it seems safer to return
** false.
*/
int sqlite3FitsIn64Bits(const char *zNum){
  int i, c;
  if( *zNum=='-' || *zNum=='+' ) zNum++;
  for(i=0; (c=zNum[i])>='0' && c<='9'; i++){}
  return i<19 || (i==19 && memcmp(zNum,"9223372036854775807",19)<=0);
}


/*
** Change the sqlite.magic from SQLITE_MAGIC_OPEN to SQLITE_MAGIC_BUSY.
** Return an error (non-zero) if the magic was not SQLITE_MAGIC_OPEN
** when this routine is called.
**
** This routine is a attempt to detect if two threads use the
** same sqlite* pointer at the same time.  There is a race 
** condition so it is possible that the error is not detected.
** But usually the problem will be seen.  The result will be an
** error which can be used to debug the application that is
** using SQLite incorrectly.
**
** Ticket #202:  If db->magic is not a valid open value, take care not
** to modify the db structure at all.  It could be that db is a stale
** pointer.  In other words, it could be that there has been a prior
** call to sqlite3_close(db) and db has been deallocated.  And we do
** not want to write into deallocated memory.
*/
int sqlite3SafetyOn(sqlite3 *db){
  if( db->magic==SQLITE_MAGIC_OPEN ){
    db->magic = SQLITE_MAGIC_BUSY;
    return 0;
  }else if( db->magic==SQLITE_MAGIC_BUSY ){
    db->magic = SQLITE_MAGIC_ERROR;
    db->flags |= SQLITE_Interrupt;
  }
  return 1;
}

/*
** Change the magic from SQLITE_MAGIC_BUSY to SQLITE_MAGIC_OPEN.
** Return an error (non-zero) if the magic was not SQLITE_MAGIC_BUSY
** when this routine is called.
*/
int sqlite3SafetyOff(sqlite3 *db){
  if( db->magic==SQLITE_MAGIC_BUSY ){
    db->magic = SQLITE_MAGIC_OPEN;
    return 0;
  }else if( db->magic==SQLITE_MAGIC_OPEN ){
    db->magic = SQLITE_MAGIC_ERROR;
    db->flags |= SQLITE_Interrupt;
  }
  return 1;
}

/*
** Check to make sure we have a valid db pointer.  This test is not
** foolproof but it does provide some measure of protection against
** misuse of the interface such as passing in db pointers that are
** NULL or which have been previously closed.  If this routine returns
** TRUE it means that the db pointer is invalid and should not be
** dereferenced for any reason.  The calling function should invoke
** SQLITE_MISUSE immediately.
*/
int sqlite3SafetyCheck(sqlite3 *db){
  int magic;
  if( db==0 ) return 1;
  magic = db->magic;
  if( magic!=SQLITE_MAGIC_CLOSED &&
         magic!=SQLITE_MAGIC_OPEN &&
         magic!=SQLITE_MAGIC_BUSY ) return 1;
  return 0;
}

/*
** The variable-length integer encoding is as follows:
**
** KEY:
**         A = 0xxxxxxx    7 bits of data and one flag bit
**         B = 1xxxxxxx    7 bits of data and one flag bit
**         C = xxxxxxxx    8 bits of data
**
**  7 bits - A
** 14 bits - BA
** 21 bits - BBA
** 28 bits - BBBA
** 35 bits - BBBBA
** 42 bits - BBBBBA
** 49 bits - BBBBBBA
** 56 bits - BBBBBBBA
** 64 bits - BBBBBBBBC
*/

/*
** Write a 64-bit variable-length integer to memory starting at p[0].
** The length of data write will be between 1 and 9 bytes.  The number
** of bytes written is returned.
**
** A variable-length integer consists of the lower 7 bits of each byte
** for all bytes that have the 8th bit set and one byte with the 8th
** bit clear.  Except, if we get to the 9th byte, it stores the full
** 8 bits and is the last byte.
*/
int sqlite3PutVarint(unsigned char *p, u64 v){
  int i, j, n;
  u8 buf[10];
  if( v & (((u64)0xff000000)<<32) ){
    p[8] = v;
    v >>= 8;
    for(i=7; i>=0; i--){
      p[i] = (v & 0x7f) | 0x80;
      v >>= 7;
    }
    return 9;
  }    
  n = 0;
  do{
    buf[n++] = (v & 0x7f) | 0x80;
    v >>= 7;
  }while( v!=0 );
  buf[0] &= 0x7f;
  assert( n<=9 );
  for(i=0, j=n-1; j>=0; j--, i++){
    p[i] = buf[j];
  }
  return n;
}

/*
** Read a 64-bit variable-length integer from memory starting at p[0].
** Return the number of bytes read.  The value is stored in *v.
*/
int sqlite3GetVarint(const unsigned char *p, u64 *v){
  u32 x;
  u64 x64;
  int n;
  unsigned char c;
  if( ((c = p[0]) & 0x80)==0 ){
    *v = c;
    return 1;
  }
  x = c & 0x7f;
  if( ((c = p[1]) & 0x80)==0 ){
    *v = (x<<7) | c;
    return 2;
  }
  x = (x<<7) | (c&0x7f);
  if( ((c = p[2]) & 0x80)==0 ){
    *v = (x<<7) | c;
    return 3;
  }
  x = (x<<7) | (c&0x7f);
  if( ((c = p[3]) & 0x80)==0 ){
    *v = (x<<7) | c;
    return 4;
  }
  x64 = (x<<7) | (c&0x7f);
  n = 4;
  do{
    c = p[n++];
    if( n==9 ){
      x64 = (x64<<8) | c;
      break;
    }
    x64 = (x64<<7) | (c&0x7f);
  }while( (c & 0x80)!=0 );
  *v = x64;
  return n;
}

/*
** Read a 32-bit variable-length integer from memory starting at p[0].
** Return the number of bytes read.  The value is stored in *v.
*/
int sqlite3GetVarint32(const unsigned char *p, u32 *v){
  u32 x;
  int n;
  unsigned char c;
  if( ((signed char*)p)[0]>=0 ){
    *v = p[0];
    return 1;
  }
  x = p[0] & 0x7f;
  if( ((signed char*)p)[1]>=0 ){
    *v = (x<<7) | p[1];
    return 2;
  }
  x = (x<<7) | (p[1] & 0x7f);
  n = 2;
  do{
    x = (x<<7) | ((c = p[n++])&0x7f);
  }while( (c & 0x80)!=0 && n<9 );
  *v = x;
  return n;
}

/*
** Return the number of bytes that will be needed to store the given
** 64-bit integer.
*/
int sqlite3VarintLen(u64 v){
  int i = 0;
  do{
    i++;
    v >>= 7;
  }while( v!=0 && i<9 );
  return i;
}

#if !defined(SQLITE_OMIT_BLOB_LITERAL) || defined(SQLITE_HAS_CODEC) \
    || defined(SQLITE_TEST)
/*
** Translate a single byte of Hex into an integer.
*/
static int hexToInt(int h){
  if( h>='0' && h<='9' ){
    return h - '0';
  }else if( h>='a' && h<='f' ){
    return h - 'a' + 10;
  }else{
    assert( h>='A' && h<='F' );
    return h - 'A' + 10;
  }
}
#endif /* !SQLITE_OMIT_BLOB_LITERAL || SQLITE_HAS_CODEC || SQLITE_TEST */

#if !defined(SQLITE_OMIT_BLOB_LITERAL) || defined(SQLITE_HAS_CODEC)
/*
** Convert a BLOB literal of the form "x'hhhhhh'" into its binary
** value.  Return a pointer to its binary value.  Space to hold the
** binary value has been obtained from malloc and must be freed by
** the calling routine.
*/
void *sqlite3HexToBlob(const char *z){
  char *zBlob;
  int i;
  int n = strlen(z);
  if( n%2 ) return 0;

  zBlob = (char *)sqliteMalloc(n/2);
  for(i=0; i<n; i+=2){
    zBlob[i/2] = (hexToInt(z[i])<<4) | hexToInt(z[i+1]);
  }
  return zBlob;
}
#endif /* !SQLITE_OMIT_BLOB_LITERAL || SQLITE_HAS_CODEC */

#if defined(SQLITE_TEST)
/*
** Convert text generated by the "%p" conversion format back into
** a pointer.
*/
void *sqlite3TextToPtr(const char *z){
  void *p;
  u64 v;
  u32 v2;
  if( z[0]=='0' && z[1]=='x' ){
    z += 2;
  }
  v = 0;
  while( *z ){
    v = (v<<4) + hexToInt(*z);
    z++;
  }
  if( sizeof(p)==sizeof(v) ){
    p = *(void**)&v;
  }else{
    assert( sizeof(p)==sizeof(v2) );
    v2 = (u32)v;
    p = *(void**)&v2;
  }
  return p;
}
#endif

/*
** Return a pointer to the ThreadData associated with the calling thread.
*/
ThreadData *sqlite3ThreadData(){
  ThreadData *p = (ThreadData*)sqlite3OsThreadSpecificData(1);
  if( !p ){
    sqlite3FailedMalloc();
  }
  return p;
}

/*
** Return a pointer to the ThreadData associated with the calling thread.
** If no ThreadData has been allocated to this thread yet, return a pointer
** to a substitute ThreadData structure that is all zeros. 
*/
const ThreadData *sqlite3ThreadDataReadOnly(){
  static const ThreadData zeroData = {0};  /* Initializer to silence warnings
                                           ** from broken compilers */
  const ThreadData *pTd = sqlite3OsThreadSpecificData(0);
  return pTd ? pTd : &zeroData;
}

/*
** Check to see if the ThreadData for this thread is all zero.  If it
** is, then deallocate it. 
*/
void sqlite3ReleaseThreadData(){
  sqlite3OsThreadSpecificData(-1);
}

/*
** This function must be called before exiting any API function (i.e. 
** returning control to the user) that has called sqlite3Malloc or
** sqlite3Realloc.
**
** The returned value is normally a copy of the second argument to this
** function. However, if a malloc() failure has occured since the previous
** invocation SQLITE_NOMEM is returned instead. 
**
** If the first argument, db, is not NULL and a malloc() error has occured,
** then the connection error-code (the value returned by sqlite3_errcode())
** is set to SQLITE_NOMEM.
*/
static int mallocHasFailed = 0;
/* int sqlite3ApiExit(sqlite3* db, int rc){ */
/*   if( sqlite3MallocFailed() ){ */
/*     mallocHasFailed = 0; */
/*     sqlite3OsLeaveMutex(); */
/*     sqlite3Error(db, SQLITE_NOMEM, 0); */
/*     rc = SQLITE_NOMEM; */
/*   } */
/*   return rc; */
/* } */

/* 
** Return true is a malloc has failed in this thread since the last call
** to sqlite3ApiExit(), or false otherwise.
*/
int sqlite3MallocFailed(){
  return (mallocHasFailed && sqlite3OsInMutex(1));
}

/* 
** Set the "malloc has failed" condition to true for this thread.
*/
void sqlite3FailedMalloc(){
  sqlite3OsEnterMutex();
  assert( mallocHasFailed==0 );
  mallocHasFailed = 1;
}

#ifdef SQLITE_MEMDEBUG
/*
** This function sets a flag in the thread-specific-data structure that will
** cause an assert to fail if sqliteMalloc() or sqliteRealloc() is called.
*/
void sqlite3MallocDisallow(){
  assert( sqlite3_mallocDisallowed>=0 );
  sqlite3_mallocDisallowed++;
}

/*
** This function clears the flag set in the thread-specific-data structure set
** by sqlite3MallocDisallow().
*/
void sqlite3MallocAllow(){
  assert( sqlite3_mallocDisallowed>0 );
  sqlite3_mallocDisallowed--;
}
#endif

/*
** Return a static string that describes the kind of error specified in the
** argument.
*/
const char *sqlite3ErrStr(int rc){
  const char *z;
  switch( rc ){
    case SQLITE_ROW:
    case SQLITE_DONE:
    case SQLITE_OK:         z = "not an error";                          break;
    case SQLITE_ERROR:      z = "SQL logic error or missing database";   break;
    case SQLITE_PERM:       z = "access permission denied";              break;
    case SQLITE_ABORT:      z = "callback requested query abort";        break;
    case SQLITE_BUSY:       z = "database is locked";                    break;
    case SQLITE_LOCKED:     z = "database table is locked";              break;
    case SQLITE_NOMEM:      z = "out of memory";                         break;
    case SQLITE_READONLY:   z = "attempt to write a readonly database";  break;
    case SQLITE_INTERRUPT:  z = "interrupted";                           break;
    case SQLITE_IOERR:      z = "disk I/O error";                        break;
    case SQLITE_CORRUPT:    z = "database disk image is malformed";      break;
    case SQLITE_FULL:       z = "database or disk is full";              break;
    case SQLITE_CANTOPEN:   z = "unable to open database file";          break;
    case SQLITE_PROTOCOL:   z = "database locking protocol failure";     break;
    case SQLITE_EMPTY:      z = "table contains no data";                break;
    case SQLITE_SCHEMA:     z = "database schema has changed";           break;
    case SQLITE_CONSTRAINT: z = "constraint failed";                     break;
    case SQLITE_MISMATCH:   z = "datatype mismatch";                     break;
    case SQLITE_MISUSE:     z = "library routine called out of sequence";break;
    case SQLITE_NOLFS:      z = "kernel lacks large file support";       break;
    case SQLITE_AUTH:       z = "authorization denied";                  break;
    case SQLITE_FORMAT:     z = "auxiliary database format error";       break;
    case SQLITE_RANGE:      z = "bind or column index out of range";     break;
    case SQLITE_NOTADB:     z = "file is encrypted or is not a database";break;
    default:                z = "unknown error";                         break;
  }
  return z;
}

/* mock sqlite3IsLikeFunction, the real version is in func.c:1130*/
//int sqlite3IsLikeFunction(sqlite3 *db, Expr *pExpr, int *pIsNocase, char *aWc){}

void resetParseObject(Parse *parseObj) {
    if (parseObj != NULL) {
        sqlite3ParseReset(parseObj);
    }
}

Parse* sqlite3ParseNew() {
    Parse *pNew = NULL;
    pNew = (Parse*) sqliteMalloc(sizeof(*pNew));
    if (pNew == NULL) { return NULL; }
    
    return sqlite3ParseInit(pNew);
}

Parse* sqlite3ParseInit(Parse *parseObj) {
    if (parseObj == NULL) { return parseObj; }   
    parseObj->tokens.allocSize = 16;// init size
    parseObj->tokens.array = (TokenItem*)sqliteMalloc(sizeof(parseObj->tokens.array[0]) * parseObj->tokens.allocSize);
    if (parseObj->tokens.array == NULL) {
        sqliteFree(parseObj);
        return NULL;
    }
    parseObj->tokens.curSize = 0;
    return parseObj;
}

/**
 * not free the tokens.array or parsed.array, just set it's size to 0
 * free it, please use sqlite3ParseDelete
 */
void sqlite3ParseReset(Parse *parseObj) {
    if (parseObj == NULL) { return; }
    
    TokenArray tokens = parseObj->tokens;
    ParsedResultArray parsed = parseObj->parsed;

    memset(parseObj, 0, sizeof(*parseObj));
    parseObj->tokens = tokens; // restore tokens
    parseObj->tokens.curSize = 0;
    parseObj->parsed = parsed;
    sqlite3ParsedResultArrayClean(&parseObj->parsed);
}

void sqlite3ParseDelete(Parse *parseObj) {
    if (parseObj == NULL) { return; }

    if (parseObj->tokens.array) {
        sqliteFree(parseObj->tokens.array);
    }

    if (parseObj->parsed.array) {
        sqlite3ParsedResultArrayClean(&parseObj->parsed);
        sqliteFree(parseObj->parsed.array);
    }

    sqliteFree(parseObj);
}

TokenArray* sqlite3TokenArrayAppend(TokenArray *tokenArray, TokenItem *item) {
    if (!tokenArray || !tokenArray->array || !item) { return tokenArray; }

    if (tokenArray->curSize >= tokenArray->allocSize) {
        TokenItem *array = NULL;
        u32 allocSize = tokenArray->allocSize * 2;
        array = (TokenItem*)sqliteRealloc(tokenArray->array, allocSize * sizeof(tokenArray->array[0]));
        if (array == NULL) {
            return NULL;
        }

        tokenArray->array = array;
        tokenArray->allocSize = allocSize;
    }

    tokenArray->array[tokenArray->curSize++] = *item;
    return tokenArray;
}

ParsedResultArray* sqlite3ParsedResultArrayAppend(ParsedResultArray *resultArray, ParsedResultItem *result) {
    if (!resultArray || !result) { return resultArray; }

    if (resultArray->array == NULL) {
        u32 initSize = 1;
        ParsedResultItem *array = (ParsedResultItem*)sqliteMalloc(initSize * sizeof(resultArray->array[0]));
        if (array == NULL) {
            return NULL;
        }

        resultArray->array = array;
        resultArray->allocSize = initSize;
    }   

    if (resultArray->curSize >= resultArray->allocSize) {
        u32 allocSize = resultArray->allocSize * 2;
        ParsedResultItem *array = (ParsedResultItem*) sqliteRealloc(resultArray->array, allocSize * sizeof(resultArray->array[0]));
        if (array == NULL) {
            return NULL;
        }
        resultArray->array = array;
        resultArray->allocSize = allocSize;
    }

    resultArray->array[resultArray->curSize++] = *result;
    return resultArray;
}

static void sqlite3ParsedItemFree(ParsedResultItem *parsedItem) {
    switch(parsedItem->sqltype) {
        case SQLTYPE_SELECT:
            sqlite3SelectDelete(parsedItem->result.selectObj);
            break;

        case SQLTYPE_REPLACE:
        case SQLTYPE_INSERT:
            sqlite3InsertDelete(parsedItem->result.insertObj);
            break;

        case SQLTYPE_UPDATE:
            sqlite3UpdateDelete(parsedItem->result.updateObj);
            break;

        case SQLTYPE_DELETE:
            sqlite3DeleteFree(parsedItem->result.deleteObj);
            break;
        
        case SQLTYPE_SET:
        case SQLTYPE_SET_NAMES:
        case SQLTYPE_SET_CHARACTER_SET:
            sqlite3SetStatementDelete(parsedItem->result.setObj);
            break;           
            
        default:
            break;
    }
}

void sqlite3ParsedResultArrayClean(ParsedResultArray *resultArray) {
     if (!resultArray || !resultArray->array || resultArray->curSize <= 0) { return; }
    
     u32 i;
     for (i = 0; i < resultArray->curSize; i++) {
        sqlite3ParsedItemFree(&resultArray->array[i]);
     }
    resultArray->curSize = 0;
}
/****************************file from where.c********************************************/
/*
** The number of bits in a Bitmask.  "BMS" means "BitMask Size".
*/
#define BMS  (sizeof(Bitmask)*8)

/*
** Determine the number of elements in an array.
*/
#define ARRAYSIZE(X)  (sizeof(X)/sizeof(X[0]))

/*
** Trace output macros
*/
#if defined(SQLITE_TEST) || defined(SQLITE_DEBUG)
int sqlite3_where_trace = 0;
# define TRACE(X)  if(sqlite3_where_trace) sqlite3DebugPrintf X
#else
# define TRACE(X)
#endif

/* Forward reference
*/
typedef struct WhereClause WhereClause;

typedef struct WhereTerm WhereTerm;
struct WhereTerm {
  Expr *pExpr;            /* Pointer to the subexpression */
  i16 iParent;            /* Disable pWC->a[iParent] when this term disabled */
  i16 leftCursor;         /* Cursor number of X in "X <op> <expr>" */
  i16 leftColumn;         /* Column number of X in "X <op> <expr>" */
  u16 eOperator;          /* A WO_xx value describing <op> */
  u8 flags;               /* Bit flags.  See below */
  u8 nChild;              /* Number of children that must disable us */
  WhereClause *pWC;       /* The clause this term is part of */
  Bitmask prereqRight;    /* Bitmask of tables used by pRight */
  Bitmask prereqAll;      /* Bitmask of tables referenced by p */
};

/*
** Allowed values of WhereTerm.flags
*/
#define TERM_DYNAMIC    0x01   /* Need to call sqlite3ExprDelete(pExpr) */
#define TERM_VIRTUAL    0x02   /* Added by the optimizer.  Do not code */
#define TERM_CODED      0x04   /* This term is already coded */
#define TERM_COPIED     0x08   /* Has a child */
#define TERM_OR_OK      0x10   /* Used during OR-clause processing */

/*
** An instance of the following structure holds all information about a
** WHERE clause.  Mostly this is a container for one or more WhereTerms.
*/
struct WhereClause {
  Parse *pParse;           /* The parser context */
  int nTerm;               /* Number of terms */
  int nSlot;               /* Number of entries in a[] */
  WhereTerm *a;            /* Each a[] describes a term of the WHERE cluase */
  WhereTerm aStatic[10];   /* Initial static space for a[] */
};

typedef struct ExprMaskSet ExprMaskSet;
struct ExprMaskSet {
  int n;                        /* Number of assigned cursor values */
  int ix[sizeof(Bitmask)*8];    /* Cursor assigned to each bit */
};

/*
** Bitmasks for the operators that indices are able to exploit.  An
** OR-ed combination of these values can be used when searching for
** terms in the where clause.
*/
#define WO_IN     1
#define WO_EQ     2
#define WO_LT     (WO_EQ<<(TK_LT-TK_EQ))
#define WO_LE     (WO_EQ<<(TK_LE-TK_EQ))
#define WO_GT     (WO_EQ<<(TK_GT-TK_EQ))
#define WO_GE     (WO_EQ<<(TK_GE-TK_EQ))

/*
** Value for flags returned by bestIndex()
*/
#define WHERE_ROWID_EQ       0x0001   /* rowid=EXPR or rowid IN (...) */
#define WHERE_ROWID_RANGE    0x0002   /* rowid<EXPR and/or rowid>EXPR */
#define WHERE_COLUMN_EQ      0x0010   /* x=EXPR or x IN (...) */
#define WHERE_COLUMN_RANGE   0x0020   /* x<EXPR and/or x>EXPR */
#define WHERE_COLUMN_IN      0x0040   /* x IN (...) */
#define WHERE_TOP_LIMIT      0x0100   /* x<EXPR or x<=EXPR constraint */
#define WHERE_BTM_LIMIT      0x0200   /* x>EXPR or x>=EXPR constraint */
#define WHERE_IDX_ONLY       0x0800   /* Use index only - omit table */
#define WHERE_ORDERBY        0x1000   /* Output will appear in correct order */
#define WHERE_REVERSE        0x2000   /* Scan in reverse order */
#define WHERE_UNIQUE         0x4000   /* Selects no more than one row */

/*
** Initialize a preallocated WhereClause structure.
*/
static void whereClauseInit(WhereClause *pWC, Parse *pParse){
  pWC->pParse = pParse;
  pWC->nTerm = 0;
  pWC->nSlot = ARRAYSIZE(pWC->aStatic);
  pWC->a = pWC->aStatic;
}

/*
** Deallocate a WhereClause structure.  The WhereClause structure
** itself is not freed.  This routine is the inverse of whereClauseInit().
*/
static void whereClauseClear(WhereClause *pWC){
  int i;
  WhereTerm *a;
  for(i=pWC->nTerm-1, a=pWC->a; i>=0; i--, a++){
    if( a->flags & TERM_DYNAMIC ){
      sqlite3ExprDelete(a->pExpr);
    }
  }
  if( pWC->a!=pWC->aStatic ){
    sqliteFree(pWC->a);
  }
}

/*
** Add a new entries to the WhereClause structure.  Increase the allocated
** space as necessary.
**
** WARNING:  This routine might reallocate the space used to store
** WhereTerms.  All pointers to WhereTerms should be invalided after
** calling this routine.  Such pointers may be reinitialized by referencing
** the pWC->a[] array.
*/
static int whereClauseInsert(WhereClause *pWC, Expr *p, int flags){
  WhereTerm *pTerm;
  int idx;
  if( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    pWC->a = sqliteMalloc( sizeof(pWC->a[0])*pWC->nSlot*2 );
    if( pWC->a==0 ) return 0;
    memcpy(pWC->a, pOld, sizeof(pWC->a[0])*pWC->nTerm);
    if( pOld!=pWC->aStatic ){
      sqliteFree(pOld);
    }
    pWC->nSlot *= 2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm];
  pWC->nTerm++;
  pTerm->pExpr = p;
  pTerm->flags = flags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  return idx;
}

static void whereSplit(WhereClause *pWC, Expr *pExpr, int op){
  if( pExpr==0 ) return;
  if( pExpr->op!=op ){
    whereClauseInsert(pWC, pExpr, 0);
  }else{
    whereSplit(pWC, pExpr->pLeft, op);
    whereSplit(pWC, pExpr->pRight, op);
  }
}

/*
** Initialize an expression mask set
*/
#define initMaskSet(P)  memset(P, 0, sizeof(*P))

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
static Bitmask getMask(ExprMaskSet *pMaskSet, int iCursor){
  int i;
  for(i=0; i<pMaskSet->n; i++){
    if( pMaskSet->ix[i]==iCursor ){
      return ((Bitmask)1)<<i;
    }
  }
  return 0;
}

/*
** Create a new mask for cursor iCursor.
**
** There is one cursor per table in the FROM clause.  The number of
** tables in the FROM clause is limited by a test early in the
** sqlite3WhereBegin() routine.  So we know that the pMaskSet->ix[]
** array will never overflow.
*/
static void createMask(ExprMaskSet *pMaskSet, int iCursor){
  assert( pMaskSet->n < ARRAYSIZE(pMaskSet->ix) );
  pMaskSet->ix[pMaskSet->n++] = iCursor;
}

/*
** This routine walks (recursively) an expression tree and generates
** a bitmask indicating which tables are used in that expression
** tree.
**
** In order for this routine to work, the calling function must have
** previously invoked sqlite3ExprResolveNames() on the expression.  See
** the header comment on that routine for additional information.
** The sqlite3ExprResolveNames() routines looks for column names and
** sets their opcodes to TK_COLUMN and their Expr.iTable fields to
** the VDBE cursor number of the table.  This routine just has to
** translate the cursor numbers into bitmask values and OR all
** the bitmasks together.
*/
static Bitmask exprListTableUsage(ExprMaskSet*, ExprList*);
static Bitmask exprSelectTableUsage(ExprMaskSet*, Select*);
static Bitmask exprTableUsage(ExprMaskSet *pMaskSet, Expr *p){
  Bitmask mask = 0;
  if( p==0 ) return 0;
  if( p->op==TK_COLUMN ){
    mask = getMask(pMaskSet, p->iTable);
    return mask;
  }
  mask = exprTableUsage(pMaskSet, p->pRight);
  mask |= exprTableUsage(pMaskSet, p->pLeft);
  mask |= exprListTableUsage(pMaskSet, p->pList);
  mask |= exprSelectTableUsage(pMaskSet, p->pSelect);
  return mask;
}
static Bitmask exprListTableUsage(ExprMaskSet *pMaskSet, ExprList *pList){
  int i;
  Bitmask mask = 0;
  if( pList ){
    for(i=0; i<pList->nExpr; i++){
      mask |= exprTableUsage(pMaskSet, pList->a[i].pExpr);
    }
  }
  return mask;
}
static Bitmask exprSelectTableUsage(ExprMaskSet *pMaskSet, Select *pS){
  Bitmask mask;
  if( pS==0 ){
    mask = 0;
  }else{
    mask = exprListTableUsage(pMaskSet, pS->pEList);
    mask |= exprListTableUsage(pMaskSet, pS->pGroupBy);
    mask |= exprListTableUsage(pMaskSet, pS->pOrderBy);
    mask |= exprTableUsage(pMaskSet, pS->pWhere);
    mask |= exprTableUsage(pMaskSet, pS->pHaving);
  }
  return mask;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", and "IN".
*/
static int allowedOp(int op){
  assert( TK_GT>TK_EQ && TK_GT<TK_GE );
  assert( TK_LT>TK_EQ && TK_LT<TK_GE );
  assert( TK_LE>TK_EQ && TK_LE<TK_GE );
  assert( TK_GE==TK_EQ+4 );
  return op==TK_IN || (op>=TK_EQ && op<=TK_GE);
}

/*
** Swap two objects of type T.
*/
#define SWAP(TYPE,A,B) {TYPE t=A; A=B; B=t;}

/*
** Commute a comparision operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
static void exprCommute(Expr *pExpr){
  assert( allowedOp(pExpr->op) && pExpr->op!=TK_IN );
  SWAP(CollSeq*,pExpr->pRight->pColl,pExpr->pLeft->pColl);
  SWAP(Expr*,pExpr->pRight,pExpr->pLeft);
  if( pExpr->op>=TK_GT ){
    assert( TK_LT==TK_GT+2 );
    assert( TK_GE==TK_LE+2 );
    assert( TK_GT>TK_EQ );
    assert( TK_GT<TK_LE );
    assert( pExpr->op>=TK_GT && pExpr->op<=TK_GE );
    pExpr->op = ((pExpr->op-TK_GT)^2)+TK_GT;
  }
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
static int operatorMask(int op){
  int c;
  assert( allowedOp(op) );
  if( op==TK_IN ){
    c = WO_IN;
  }else{
    c = WO_EQ<<(op-TK_EQ);
  }
  assert( op!=TK_IN || c==WO_IN );
  assert( op!=TK_EQ || c==WO_EQ );
  assert( op!=TK_LT || c==WO_LT );
  assert( op!=TK_LE || c==WO_LE );
  assert( op!=TK_GT || c==WO_GT );
  assert( op!=TK_GE || c==WO_GE );
  return c;
}

/*
** Search for a term in the WHERE clause that is of the form "X <op> <expr>"
** where X is a reference to the iColumn of table iCur and <op> is one of
** the WO_xx operator codes specified by the op parameter.
** Return a pointer to the term.  Return 0 if not found.
*/
/* static WhereTerm *findTerm( */
/*   WhereClause *pWC,     /1* The WHERE clause to be searched *1/ */
/*   int iCur,             /1* Cursor number of LHS *1/ */
/*   int iColumn,          /1* Column number of LHS *1/ */
/*   Bitmask notReady,     /1* RHS must not overlap with this mask *1/ */
/*   u16 op,               /1* Mask of WO_xx values describing operator *1/ */
/*   Index *pIdx           /1* Must be compatible with this index, if not NULL *1/ */
/* ){ */
/*   WhereTerm *pTerm; */
/*   int k; */
/*   for(pTerm=pWC->a, k=pWC->nTerm; k; k--, pTerm++){ */
/*     if( pTerm->leftCursor==iCur */
/*        && (pTerm->prereqRight & notReady)==0 */
/*        && pTerm->leftColumn==iColumn */
/*        && (pTerm->eOperator & op)!=0 */
/*     ){ */
/*       if( iCur>=0 && pIdx ){ */
/*         Expr *pX = pTerm->pExpr; */
/*         CollSeq *pColl; */
/*         char idxaff; */
/*         int j; */
/*         Parse *pParse = pWC->pParse; */

/*         idxaff = pIdx->pTable->aCol[iColumn].affinity; */
/*         if( !sqlite3IndexAffinityOk(pX, idxaff) ) continue; */
/*         pColl = sqlite3ExprCollSeq(pParse, pX->pLeft); */
/*         if( !pColl ){ */
/*           if( pX->pRight ){ */
/*             pColl = sqlite3ExprCollSeq(pParse, pX->pRight); */
/*           } */
/*           if( !pColl ){ */
/*             pColl = pParse->db->pDfltColl; */
/*           } */
/*         } */
/*         for(j=0; j<pIdx->nColumn && pIdx->aiColumn[j]!=iColumn; j++){} */
/*         assert( j<pIdx->nColumn ); */
/*         if( sqlite3StrICmp(pColl->zName, pIdx->azColl[j]) ) continue; */
/*       } */
/*       return pTerm; */
/*     } */
/*   } */
/*   return 0; */
/* } */

/* Forward reference */
static void exprAnalyze(SrcList*, ExprMaskSet*, WhereClause*, int);

/*
** Call exprAnalyze on all terms in a WHERE clause.  
**
**
*/
static void exprAnalyzeAll(
  SrcList *pTabList,       /* the FROM clause */
  ExprMaskSet *pMaskSet,   /* table masks */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  int i;
  for(i=pWC->nTerm-1; i>=0; i--){
    exprAnalyze(pTabList, pMaskSet, pWC, i);
  }
}

#ifndef SQLITE_OMIT_LIKE_OPTIMIZATION
/*
** Check to see if the given expression is a LIKE or GLOB operator that
** can be optimized using inequality constraints.  Return TRUE if it is
** so and false if not.
**
** In order for the operator to be optimizible, the RHS must be a string
** literal that does not begin with a wildcard.  
*/
/* static int isLikeOrGlob( */
/*   sqlite3 *db,      /1* The database *1/ */
/*   Expr *pExpr,      /1* Test this expression *1/ */
/*   int *pnPattern,   /1* Number of non-wildcard prefix characters *1/ */
/*   int *pisComplete  /1* True if the only wildcard is % in the last character *1/ */
/* ){ */
/*   const char *z; */
/*   Expr *pRight, *pLeft; */
/*   ExprList *pList; */
/*   int c, cnt; */
/*   int noCase; */
/*   char wc[3]; */
/*   CollSeq *pColl; */

/*   if( !sqlite3IsLikeFunction(db, pExpr, &noCase, wc) ){ */
/*     return 0; */
/*   } */
/*   pList = pExpr->pList; */
/*   pRight = pList->a[0].pExpr; */
/*   if( pRight->op!=TK_STRING ){ */
/*     return 0; */
/*   } */
/*   pLeft = pList->a[1].pExpr; */
/*   if( pLeft->op!=TK_COLUMN ){ */
/*     return 0; */
/*   } */
/*   pColl = pLeft->pColl; */
/*   if( pColl==0 ){ */
/*     pColl = db->pDfltColl; */
/*   } */
/*   if( (pColl->type!=SQLITE_COLL_BINARY || noCase) && */
/*       (pColl->type!=SQLITE_COLL_NOCASE || !noCase) ){ */
/*     return 0; */
/*   } */
/*   sqlite3DequoteExpr(pRight); */
/*   z = (char *)pRight->token.z; */
/*   for(cnt=0; (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2]; cnt++){} */
/*   if( cnt==0 || 255==(u8)z[cnt] ){ */
/*     return 0; */
/*   } */
/*   *pisComplete = z[cnt]==wc[0] && z[cnt+1]==0; */
/*   *pnPattern = cnt; */
/*   return 1; */
/* } */
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
/* static void transferJoinMarkings(Expr *pDerived, Expr *pBase){ */
/*   pDerived->flags |= pBase->flags & EP_FromJoin; */
/*   pDerived->iRightJoinTable = pBase->iRightJoinTable; */
/* } */


/*
** The input to this routine is an WhereTerm structure with only the
** "pExpr" field filled in.  The job of this routine is to analyze the
** subexpression and populate all the other fields of the WhereTerm
** structure.
**
** If the expression is of the form "<expr> <op> X" it gets commuted
** to the standard form of "X <op> <expr>".  If the expression is of
** the form "X <op> Y" where both X and Y are columns, then the original
** expression is unchanged and a new virtual expression of the form
** "Y <op> X" is added to the WHERE clause and analyzed separately.
*/
static void exprAnalyze(
  SrcList *pSrc,            /* the FROM clause */
  ExprMaskSet *pMaskSet,    /* table masks */
  WhereClause *pWC,         /* the WHERE clause */
  int idxTerm               /* Index of the term to be analyzed */
){
  /* WhereTerm *pTerm = &pWC->a[idxTerm]; */
  /* Expr *pExpr = pTerm->pExpr; */
  /* Bitmask prereqLeft; */
  /* Bitmask prereqAll; */
  /* int nPattern; */
  /* int isComplete; */

  /* if( sqlite3MallocFailed() ) return; */
  /* prereqLeft = exprTableUsage(pMaskSet, pExpr->pLeft); */
  /* if( pExpr->op==TK_IN ){ */
  /*   assert( pExpr->pRight==0 ); */
  /*   pTerm->prereqRight = exprListTableUsage(pMaskSet, pExpr->pList) */
  /*                         | exprSelectTableUsage(pMaskSet, pExpr->pSelect); */
  /* }else{ */
  /*   pTerm->prereqRight = exprTableUsage(pMaskSet, pExpr->pRight); */
  /* } */
  /* prereqAll = exprTableUsage(pMaskSet, pExpr); */
  /* if( ExprHasProperty(pExpr, EP_FromJoin) ){ */
  /*   prereqAll |= getMask(pMaskSet, pExpr->iRightJoinTable); */
  /* } */
  /* pTerm->prereqAll = prereqAll; */
  /* pTerm->leftCursor = -1; */
  /* pTerm->iParent = -1; */
  /* pTerm->eOperator = 0; */
  /* if( allowedOp(pExpr->op) && (pTerm->prereqRight & prereqLeft)==0 ){ */
  /*   Expr *pLeft = pExpr->pLeft; */
  /*   Expr *pRight = pExpr->pRight; */
  /*   if( pLeft->op==TK_COLUMN ){ */
  /*     pTerm->leftCursor = pLeft->iTable; */
  /*     pTerm->leftColumn = pLeft->iColumn; */
  /*     pTerm->eOperator = operatorMask(pExpr->op); */
  /*   } */
  /*   if( pRight && pRight->op==TK_COLUMN ){ */
  /*     WhereTerm *pNew; */
  /*     Expr *pDup; */
  /*     if( pTerm->leftCursor>=0 ){ */
  /*       int idxNew; */
  /*       pDup = sqlite3ExprDup(pExpr); */
  /*       idxNew = whereClauseInsert(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC); */
  /*       if( idxNew==0 ) return; */
  /*       pNew = &pWC->a[idxNew]; */
  /*       pNew->iParent = idxTerm; */
  /*       pTerm = &pWC->a[idxTerm]; */
  /*       pTerm->nChild = 1; */
  /*       pTerm->flags |= TERM_COPIED; */
  /*     }else{ */
  /*       pDup = pExpr; */
  /*       pNew = pTerm; */
  /*     } */
  /*     exprCommute(pDup); */
  /*     pLeft = pDup->pLeft; */
  /*     pNew->leftCursor = pLeft->iTable; */
  /*     pNew->leftColumn = pLeft->iColumn; */
  /*     pNew->prereqRight = prereqLeft; */
  /*     pNew->prereqAll = prereqAll; */
  /*     pNew->eOperator = operatorMask(pDup->op); */
  /*   } */
  /* } */

/* #ifndef SQLITE_OMIT_BETWEEN_OPTIMIZATION */
  /* /1* If a term is the BETWEEN operator, create two new virtual terms */
  /* ** that define the range that the BETWEEN implements. */
  /* *1/ */
  /* else if( pExpr->op==TK_BETWEEN ){ */
  /*   ExprList *pList = pExpr->pList; */
  /*   int i; */
  /*   static const u8 ops[] = {TK_GE, TK_LE}; */
  /*   assert( pList!=0 ); */
  /*   assert( pList->nExpr==2 ); */
  /*   for(i=0; i<2; i++){ */
  /*     Expr *pNewExpr; */
  /*     int idxNew; */
  /*     pNewExpr = sqlite3Expr(ops[i], sqlite3ExprDup(pExpr->pLeft), */
  /*                            sqlite3ExprDup(pList->a[i].pExpr), 0); */
  /*     idxNew = whereClauseInsert(pWC, pNewExpr, TERM_VIRTUAL|TERM_DYNAMIC); */
  /*     exprAnalyze(pSrc, pMaskSet, pWC, idxNew); */
  /*     pTerm = &pWC->a[idxTerm]; */
  /*     pWC->a[idxNew].iParent = idxTerm; */
  /*   } */
  /*   pTerm->nChild = 2; */
  /* } */
/* #endif /1* SQLITE_OMIT_BETWEEN_OPTIMIZATION *1/ */

/* #if !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY) */
  /* /1* Attempt to convert OR-connected terms into an IN operator so that */
  /* ** they can make use of indices.  Example: */
  /* ** */
  /* **      x = expr1  OR  expr2 = x  OR  x = expr3 */
  /* ** */
  /* ** is converted into */
  /* ** */
  /* **      x IN (expr1,expr2,expr3) */
  /* ** */
  /* ** This optimization must be omitted if OMIT_SUBQUERY is defined because */
  /* ** the compiler for the the IN operator is part of sub-queries. */
  /* *1/ */
  /* else if( pExpr->op==TK_OR ){ */
  /*   int ok; */
  /*   int i, j; */
  /*   int iColumn, iCursor; */
  /*   WhereClause sOr; */
  /*   WhereTerm *pOrTerm; */

  /*   assert( (pTerm->flags & TERM_DYNAMIC)==0 ); */
  /*   whereClauseInit(&sOr, pWC->pParse); */
  /*   whereSplit(&sOr, pExpr, TK_OR); */
  /*   exprAnalyzeAll(pSrc, pMaskSet, &sOr); */
  /*   assert( sOr.nTerm>0 ); */
  /*   j = 0; */
  /*   do{ */
  /*     iColumn = sOr.a[j].leftColumn; */
  /*     iCursor = sOr.a[j].leftCursor; */
  /*     ok = iCursor>=0; */
  /*     for(i=sOr.nTerm-1, pOrTerm=sOr.a; i>=0 && ok; i--, pOrTerm++){ */
  /*       if( pOrTerm->eOperator!=WO_EQ ){ */
  /*         goto or_not_possible; */
  /*       } */
  /*       if( pOrTerm->leftCursor==iCursor && pOrTerm->leftColumn==iColumn ){ */
  /*         pOrTerm->flags |= TERM_OR_OK; */
  /*       }else if( (pOrTerm->flags & TERM_COPIED)!=0 || */
  /*                   ((pOrTerm->flags & TERM_VIRTUAL)!=0 && */
  /*                    (sOr.a[pOrTerm->iParent].flags & TERM_OR_OK)!=0) ){ */
  /*         pOrTerm->flags &= ~TERM_OR_OK; */
  /*       }else{ */
  /*         ok = 0; */
  /*       } */
  /*     } */
  /*   }while( !ok && (sOr.a[j++].flags & TERM_COPIED)!=0 && j<sOr.nTerm ); */
  /*   if( ok ){ */
  /*     ExprList *pList = 0; */
  /*     Expr *pNew, *pDup; */
  /*     for(i=sOr.nTerm-1, pOrTerm=sOr.a; i>=0 && ok; i--, pOrTerm++){ */
  /*       if( (pOrTerm->flags & TERM_OR_OK)==0 ) continue; */
  /*       pDup = sqlite3ExprDup(pOrTerm->pExpr->pRight); */
  /*       pList = sqlite3ExprListAppend(pList, pDup, 0); */
  /*     } */
  /*     pDup = sqlite3Expr(TK_COLUMN, 0, 0, 0); */
  /*     if( pDup ){ */
  /*       pDup->iTable = iCursor; */
  /*       pDup->iColumn = iColumn; */
  /*     } */
  /*     pNew = sqlite3Expr(TK_IN, pDup, 0, 0); */
  /*     if( pNew ){ */
  /*       int idxNew; */
  /*       transferJoinMarkings(pNew, pExpr); */
  /*       pNew->pList = pList; */
  /*       idxNew = whereClauseInsert(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC); */
  /*       exprAnalyze(pSrc, pMaskSet, pWC, idxNew); */
  /*       pTerm = &pWC->a[idxTerm]; */
  /*       pWC->a[idxNew].iParent = idxTerm; */
  /*       pTerm->nChild = 1; */
  /*     }else{ */
  /*       sqlite3ExprListDelete(pList); */
  /*     } */
  /*   } */
/* or_not_possible: */
  /*   whereClauseClear(&sOr); */
  /* } */
/* #endif /1* SQLITE_OMIT_OR_OPTIMIZATION *1/ */

/* #ifndef SQLITE_OMIT_LIKE_OPTIMIZATION */
  /* /1* Add constraints to reduce the search space on a LIKE or GLOB */
  /* ** operator. */
  /* *1/ */
  /* if( isLikeOrGlob(pWC->pParse->db, pExpr, &nPattern, &isComplete) ){ */
  /*   Expr *pLeft, *pRight; */
  /*   Expr *pStr1, *pStr2; */
  /*   Expr *pNewExpr1, *pNewExpr2; */
  /*   int idxNew1, idxNew2; */

  /*   pLeft = pExpr->pList->a[1].pExpr; */
  /*   pRight = pExpr->pList->a[0].pExpr; */
  /*   pStr1 = sqlite3Expr(TK_STRING, 0, 0, 0); */
  /*   if( pStr1 ){ */
  /*     sqlite3TokenCopy(&pStr1->token, &pRight->token); */
  /*     pStr1->token.n = nPattern; */
  /*   } */
  /*   pStr2 = sqlite3ExprDup(pStr1); */
  /*   if( pStr2 ){ */
  /*     assert( pStr2->token.dyn ); */
  /*     ++*(u8*)&pStr2->token.z[nPattern-1]; */
  /*   } */
  /*   pNewExpr1 = sqlite3Expr(TK_GE, sqlite3ExprDup(pLeft), pStr1, 0); */
  /*   idxNew1 = whereClauseInsert(pWC, pNewExpr1, TERM_VIRTUAL|TERM_DYNAMIC); */
  /*   exprAnalyze(pSrc, pMaskSet, pWC, idxNew1); */
  /*   pNewExpr2 = sqlite3Expr(TK_LT, sqlite3ExprDup(pLeft), pStr2, 0); */
  /*   idxNew2 = whereClauseInsert(pWC, pNewExpr2, TERM_VIRTUAL|TERM_DYNAMIC); */
  /*   exprAnalyze(pSrc, pMaskSet, pWC, idxNew2); */
  /*   pTerm = &pWC->a[idxTerm]; */
  /*   if( isComplete ){ */
  /*     pWC->a[idxNew1].iParent = idxTerm; */
  /*     pWC->a[idxNew2].iParent = idxTerm; */
  /*     pTerm->nChild = 2; */
  /*   } */
  /* } */
/* #endif /1* SQLITE_OMIT_LIKE_OPTIMIZATION *1/ */
}


/*
** This routine decides if pIdx can be used to satisfy the ORDER BY
** clause.  If it can, it returns 1.  If pIdx cannot satisfy the
** ORDER BY clause, this routine returns 0.
**
** pOrderBy is an ORDER BY clause from a SELECT statement.  pTab is the
** left-most table in the FROM clause of that same SELECT statement and
** the table has a cursor number of "base".  pIdx is an index on pTab.
**
** nEqCol is the number of columns of pIdx that are used as equality
** constraints.  Any of these columns may be missing from the ORDER BY
** clause and the match can still be a success.
**
** All terms of the ORDER BY that match against the index must be either
** ASC or DESC.  (Terms of the ORDER BY clause past the end of a UNIQUE
** index do not need to satisfy this constraint.)  The *pbRev value is
** set to 1 if the ORDER BY clause is all DESC and it is set to 0 if
** the ORDER BY clause is all ASC.
*/
/* static int isSortingIndex( */
/*   Parse *pParse,          /1* Parsing context *1/ */
/*   Index *pIdx,            /1* The index we are testing *1/ */
/*   int base,               /1* Cursor number for the table to be sorted *1/ */
/*   ExprList *pOrderBy,     /1* The ORDER BY clause *1/ */
/*   int nEqCol,             /1* Number of index columns with == constraints *1/ */
/*   int *pbRev              /1* Set to 1 if ORDER BY is DESC *1/ */
/* ){ */
/*   int i, j;                       /1* Loop counters *1/ */
/*   int sortOrder = 0;              /1* XOR of index and ORDER BY sort direction *1/ */
/*   int nTerm;                      /1* Number of ORDER BY terms *1/ */
/*   struct ExprList_item *pTerm;    /1* A term of the ORDER BY clause *1/ */
/*   sqlite3 *db = pParse->db; */

/*   assert( pOrderBy!=0 ); */
/*   nTerm = pOrderBy->nExpr; */
/*   assert( nTerm>0 ); */

/*   /1* Match terms of the ORDER BY clause against columns of */
/*   ** the index. */
/*   *1/ */
/*   for(i=j=0, pTerm=pOrderBy->a; j<nTerm && i<pIdx->nColumn; i++){ */
/*     Expr *pExpr;       /1* The expression of the ORDER BY pTerm *1/ */
/*     CollSeq *pColl;    /1* The collating sequence of pExpr *1/ */
/*     int termSortOrder; /1* Sort order for this term *1/ */

/*     pExpr = pTerm->pExpr; */
/*     if( pExpr->op!=TK_COLUMN || pExpr->iTable!=base ){ */
/*       /1* Can not use an index sort on anything that is not a column in the */
/*       ** left-most table of the FROM clause *1/ */
/*       return 0; */
/*     } */
/*     pColl = sqlite3ExprCollSeq(pParse, pExpr); */
/*     if( !pColl ) pColl = db->pDfltColl; */
/*     if( pExpr->iColumn!=pIdx->aiColumn[i] || */ 
/*         sqlite3StrICmp(pColl->zName, pIdx->azColl[i]) ){ */
/*       /1* Term j of the ORDER BY clause does not match column i of the index *1/ */
/*       if( i<nEqCol ){ */
/*         /1* If an index column that is constrained by == fails to match an */
/*         ** ORDER BY term, that is OK.  Just ignore that column of the index */
/*         *1/ */
/*         continue; */
/*       }else{ */
/*         /1* If an index column fails to match and is not constrained by == */
/*         ** then the index cannot satisfy the ORDER BY constraint. */
/*         *1/ */
/*         return 0; */
/*       } */
/*     } */
/*     assert( pIdx->aSortOrder!=0 ); */
/*     assert( pTerm->sortOrder==0 || pTerm->sortOrder==1 ); */
/*     assert( pIdx->aSortOrder[i]==0 || pIdx->aSortOrder[i]==1 ); */
/*     termSortOrder = pIdx->aSortOrder[i] ^ pTerm->sortOrder; */
/*     if( i>nEqCol ){ */
/*       if( termSortOrder!=sortOrder ){ */
/*         /1* Indices can only be used if all ORDER BY terms past the */
/*         ** equality constraints are all either DESC or ASC. *1/ */
/*         return 0; */
/*       } */
/*     }else{ */
/*       sortOrder = termSortOrder; */
/*     } */
/*     j++; */
/*     pTerm++; */
/*   } */

/*   /1* The index can be used for sorting if all terms of the ORDER BY clause */
/*   ** are covered. */
/*   *1/ */
/*   if( j>=nTerm ){ */
/*     *pbRev = sortOrder!=0; */
/*     return 1; */
/*   } */
/*   return 0; */
/* } */

/*
** Check table to see if the ORDER BY clause in pOrderBy can be satisfied
** by sorting in order of ROWID.  Return true if so and set *pbRev to be
** true for reverse ROWID and false for forward ROWID order.
*/
static int sortableByRowid(
  int base,               /* Cursor number for table to be sorted */
  ExprList *pOrderBy,     /* The ORDER BY clause */
  int *pbRev              /* Set to 1 if ORDER BY is DESC */
){
  Expr *p;

  assert( pOrderBy!=0 );
  assert( pOrderBy->nExpr>0 );
  p = pOrderBy->a[0].pExpr;
  if( pOrderBy->nExpr==1 && p->op==TK_COLUMN && p->iTable==base
          && p->iColumn==-1 ){
    *pbRev = pOrderBy->a[0].sortOrder;
    return 1;
  }
  return 0;
}

/*
** Prepare a crude estimate of the logarithm of the input value.
** The results need not be exact.  This is only used for estimating
** the total cost of performing operatings with O(logN) or O(NlogN)
** complexity.  Because N is just a guess, it is no great tragedy if
** logN is a little off.
*/
static double estLog(double N){
  double logN = 1;
  double x = 10;
  while( N>x ){
    logN += 1;
    x *= 10;
  }
  return logN;
}

/*
** Find the best index for accessing a particular table.  Return a pointer
** to the index, flags that describe how the index should be used, the
** number of equality constraints, and the "cost" for this index.
**
** The lowest cost index wins.  The cost is an estimate of the amount of
** CPU and disk I/O need to process the request using the selected index.
** Factors that influence cost include:
**
**    *  The estimated number of rows that will be retrieved.  (The
**       fewer the better.)
**
**    *  Whether or not sorting must occur.
**
**    *  Whether or not there must be separate lookups in the
**       index and in the main table.
**
*/
/* static double bestIndex( */
/*   Parse *pParse,              /1* The parsing context *1/ */
/*   WhereClause *pWC,           /1* The WHERE clause *1/ */
/*   struct SrcList_item *pSrc,  /1* The FROM clause term to search *1/ */
/*   Bitmask notReady,           /1* Mask of cursors that are not available *1/ */
/*   ExprList *pOrderBy,         /1* The order by clause *1/ */
/*   Index **ppIndex,            /1* Make *ppIndex point to the best index *1/ */
/*   int *pFlags,                /1* Put flags describing this choice in *pFlags *1/ */
/*   int *pnEq                   /1* Put the number of == or IN constraints here *1/ */
/* ){ */
/*   WhereTerm *pTerm; */
/*   Index *bestIdx = 0;         /1* Index that gives the lowest cost *1/ */
/*   double lowestCost;          /1* The cost of using bestIdx *1/ */
/*   int bestFlags = 0;          /1* Flags associated with bestIdx *1/ */
/*   int bestNEq = 0;            /1* Best value for nEq *1/ */
/*   int iCur = pSrc->iCursor;   /1* The cursor of the table to be accessed *1/ */
/*   Index *pProbe;              /1* An index we are evaluating *1/ */
/*   int rev;                    /1* True to scan in reverse order *1/ */
/*   int flags;                  /1* Flags associated with pProbe *1/ */
/*   int nEq;                    /1* Number of == or IN constraints *1/ */
/*   double cost;                /1* Cost of using pProbe *1/ */

/*   TRACE(("bestIndex: tbl=%s notReady=%x\n", pSrc->pTab->zName, notReady)); */
/*   lowestCost = SQLITE_BIG_DBL; */
/*   pProbe = pSrc->pTab->pIndex; */

/*   /1* If the table has no indices and there are no terms in the where */
/*   ** clause that refer to the ROWID, then we will never be able to do */
/*   ** anything other than a full table scan on this table.  We might as */
/*   ** well put it first in the join order.  That way, perhaps it can be */
/*   ** referenced by other tables in the join. */
/*   *1/ */
/*   if( pProbe==0 && */
/*      findTerm(pWC, iCur, -1, 0, WO_EQ|WO_IN|WO_LT|WO_LE|WO_GT|WO_GE,0)==0 && */
/*      (pOrderBy==0 || !sortableByRowid(iCur, pOrderBy, &rev)) ){ */
/*     *pFlags = 0; */
/*     *ppIndex = 0; */
/*     *pnEq = 0; */
/*     return 0.0; */
/*   } */

/*   /1* Check for a rowid=EXPR or rowid IN (...) constraints */
/*   *1/ */
/*   pTerm = findTerm(pWC, iCur, -1, notReady, WO_EQ|WO_IN, 0); */
/*   if( pTerm ){ */
/*     Expr *pExpr; */
/*     *ppIndex = 0; */
/*     bestFlags = WHERE_ROWID_EQ; */
/*     if( pTerm->eOperator & WO_EQ ){ */
/*       /1* Rowid== is always the best pick.  Look no further.  Because only */
/*       ** a single row is generated, output is always in sorted order *1/ */
/*       *pFlags = WHERE_ROWID_EQ | WHERE_UNIQUE; */
/*       *pnEq = 1; */
/*       TRACE(("... best is rowid\n")); */
/*       return 0.0; */
/*     }else if( (pExpr = pTerm->pExpr)->pList!=0 ){ */
/*       /1* Rowid IN (LIST): cost is NlogN where N is the number of list */
/*       ** elements.  *1/ */
/*       lowestCost = pExpr->pList->nExpr; */
/*       lowestCost *= estLog(lowestCost); */
/*     }else{ */
/*       /1* Rowid IN (SELECT): cost is NlogN where N is the number of rows */
/*       ** in the result of the inner select.  We have no way to estimate */
/*       ** that value so make a wild guess. *1/ */
/*       lowestCost = 200; */
/*     } */
/*     TRACE(("... rowid IN cost: %.9g\n", lowestCost)); */
/*   } */

/*   /1* Estimate the cost of a table scan.  If we do not know how many */
/*   ** entries are in the table, use 1 million as a guess. */
/*   *1/ */
/*   cost = pProbe ? pProbe->aiRowEst[0] : 1000000; */
/*   TRACE(("... table scan base cost: %.9g\n", cost)); */
/*   flags = WHERE_ROWID_RANGE; */

/*   /1* Check for constraints on a range of rowids in a table scan. */
/*   *1/ */
/*   pTerm = findTerm(pWC, iCur, -1, notReady, WO_LT|WO_LE|WO_GT|WO_GE, 0); */
/*   if( pTerm ){ */
/*     if( findTerm(pWC, iCur, -1, notReady, WO_LT|WO_LE, 0) ){ */
/*       flags |= WHERE_TOP_LIMIT; */
/*       cost /= 3;  /1* Guess that rowid<EXPR eliminates two-thirds or rows *1/ */
/*     } */
/*     if( findTerm(pWC, iCur, -1, notReady, WO_GT|WO_GE, 0) ){ */
/*       flags |= WHERE_BTM_LIMIT; */
/*       cost /= 3;  /1* Guess that rowid>EXPR eliminates two-thirds of rows *1/ */
/*     } */
/*     TRACE(("... rowid range reduces cost to %.9g\n", cost)); */
/*   }else{ */
/*     flags = 0; */
/*   } */

/*   /1* If the table scan does not satisfy the ORDER BY clause, increase */
/*   ** the cost by NlogN to cover the expense of sorting. *1/ */
/*   if( pOrderBy ){ */
/*     if( sortableByRowid(iCur, pOrderBy, &rev) ){ */
/*       flags |= WHERE_ORDERBY|WHERE_ROWID_RANGE; */
/*       if( rev ){ */
/*         flags |= WHERE_REVERSE; */
/*       } */
/*     }else{ */
/*       cost += cost*estLog(cost); */
/*       TRACE(("... sorting increases cost to %.9g\n", cost)); */
/*     } */
/*   } */
/*   if( cost<lowestCost ){ */
/*     lowestCost = cost; */
/*     bestFlags = flags; */
/*   } */

/*   /1* Look at each index. */
/*   *1/ */
/*   for(; pProbe; pProbe=pProbe->pNext){ */
/*     int i;                       /1* Loop counter *1/ */
/*     double inMultiplier = 1; */

/*     TRACE(("... index %s:\n", pProbe->zName)); */

/*     /1* Count the number of columns in the index that are satisfied */
/*     ** by x=EXPR constraints or x IN (...) constraints. */
/*     *1/ */
/*     flags = 0; */
/*     for(i=0; i<pProbe->nColumn; i++){ */
/*       int j = pProbe->aiColumn[i]; */
/*       pTerm = findTerm(pWC, iCur, j, notReady, WO_EQ|WO_IN, pProbe); */
/*       if( pTerm==0 ) break; */
/*       flags |= WHERE_COLUMN_EQ; */
/*       if( pTerm->eOperator & WO_IN ){ */
/*         Expr *pExpr = pTerm->pExpr; */
/*         flags |= WHERE_COLUMN_IN; */
/*         if( pExpr->pSelect!=0 ){ */
/*           inMultiplier *= 100; */
/*         }else if( pExpr->pList!=0 ){ */
/*           inMultiplier *= pExpr->pList->nExpr + 1; */
/*         } */
/*       } */
/*     } */
/*     cost = pProbe->aiRowEst[i] * inMultiplier * estLog(inMultiplier); */
/*     nEq = i; */
/*     if( pProbe->onError!=OE_None && (flags & WHERE_COLUMN_IN)==0 */
/*          && nEq==pProbe->nColumn ){ */
/*       flags |= WHERE_UNIQUE; */
/*     } */
/*     TRACE(("...... nEq=%d inMult=%.9g cost=%.9g\n", nEq, inMultiplier, cost)); */

/*     /1* Look for range constraints */
/*     *1/ */
/*     if( nEq<pProbe->nColumn ){ */
/*       int j = pProbe->aiColumn[nEq]; */
/*       pTerm = findTerm(pWC, iCur, j, notReady, WO_LT|WO_LE|WO_GT|WO_GE, pProbe); */
/*       if( pTerm ){ */
/*         flags |= WHERE_COLUMN_RANGE; */
/*         if( findTerm(pWC, iCur, j, notReady, WO_LT|WO_LE, pProbe) ){ */
/*           flags |= WHERE_TOP_LIMIT; */
/*           cost /= 3; */
/*         } */
/*         if( findTerm(pWC, iCur, j, notReady, WO_GT|WO_GE, pProbe) ){ */
/*           flags |= WHERE_BTM_LIMIT; */
/*           cost /= 3; */
/*         } */
/*         TRACE(("...... range reduces cost to %.9g\n", cost)); */
/*       } */
/*     } */

/*     /1* Add the additional cost of sorting if that is a factor. */
/*     *1/ */
/*     if( pOrderBy ){ */
/*       if( (flags & WHERE_COLUMN_IN)==0 && */
/*            isSortingIndex(pParse,pProbe,iCur,pOrderBy,nEq,&rev) ){ */
/*         if( flags==0 ){ */
/*           flags = WHERE_COLUMN_RANGE; */
/*         } */
/*         flags |= WHERE_ORDERBY; */
/*         if( rev ){ */
/*           flags |= WHERE_REVERSE; */
/*         } */
/*       }else{ */
/*         cost += cost*estLog(cost); */
/*         TRACE(("...... orderby increases cost to %.9g\n", cost)); */
/*       } */
/*     } */

/*     /1* Check to see if we can get away with using just the index without */
/*     ** ever reading the table.  If that is the case, then halve the */
/*     ** cost of this index. */
/*     *1/ */
/*     if( flags && pSrc->colUsed < (((Bitmask)1)<<(BMS-1)) ){ */
/*       Bitmask m = pSrc->colUsed; */
/*       int j; */
/*       for(j=0; j<pProbe->nColumn; j++){ */
/*         int x = pProbe->aiColumn[j]; */
/*         if( x<BMS-1 ){ */
/*           m &= ~(((Bitmask)1)<<x); */
/*         } */
/*       } */
/*       if( m==0 ){ */
/*         flags |= WHERE_IDX_ONLY; */
/*         cost /= 2; */
/*         TRACE(("...... idx-only reduces cost to %.9g\n", cost)); */
/*       } */
/*     } */

/*     /1* If this index has achieved the lowest cost so far, then use it. */
/*     *1/ */
/*     if( cost < lowestCost ){ */
/*       bestIdx = pProbe; */
/*       lowestCost = cost; */
/*       assert( flags!=0 ); */
/*       bestFlags = flags; */
/*       bestNEq = nEq; */
/*     } */
/*   } */

/*   /1* Report the best result */
/*   *1/ */
/*   *ppIndex = bestIdx; */
/*   TRACE(("best index is %s, cost=%.9g, flags=%x, nEq=%d\n", */
/*         bestIdx ? bestIdx->zName : "(none)", lowestCost, bestFlags, bestNEq)); */
/*   *pFlags = bestFlags; */
/*   *pnEq = bestNEq; */
/*   return lowestCost; */
/* } */


/*
** Disable a term in the WHERE clause.  Except, do not disable the term
** if it controls a LEFT OUTER JOIN and it did not originate in the ON
** or USING clause of that join.
**
** Consider the term t2.z='ok' in the following queries:
**
**   (1)  SELECT * FROM t1 LEFT JOIN t2 ON t1.a=t2.x WHERE t2.z='ok'
**   (2)  SELECT * FROM t1 LEFT JOIN t2 ON t1.a=t2.x AND t2.z='ok'
**   (3)  SELECT * FROM t1, t2 WHERE t1.a=t2.x AND t2.z='ok'
**
** The t2.z='ok' is disabled in the in (2) because it originates
** in the ON clause.  The term is disabled in (3) because it is not part
** of a LEFT OUTER JOIN.  In (1), the term is not disabled.
**
** Disabling a term causes that term to not be tested in the inner loop
** of the join.  Disabling is an optimization.  When terms are satisfied
** by indices, we disable them to prevent redundant tests in the inner
** loop.  We would get the correct results if nothing were ever disabled,
** but joins might run a little slower.  The trick is to disable as much
** as we can without disabling too much.  If we disabled in (1), we'd get
** the wrong answer.  See ticket #813.
*/
static void disableTerm(WhereLevel *pLevel, WhereTerm *pTerm){
  if( pTerm
      && (pTerm->flags & TERM_CODED)==0
      && (pLevel->iLeftJoin==0 || ExprHasProperty(pTerm->pExpr, EP_FromJoin))
  ){
    pTerm->flags |= TERM_CODED;
    if( pTerm->iParent>=0 ){
      WhereTerm *pOther = &pTerm->pWC->a[pTerm->iParent];
      if( (--pOther->nChild)==0 ){
        disableTerm(pLevel, pOther);
      }
    }
  }
}

/*
** Generate code that builds a probe for an index.  Details:
**
**    *  Check the top nColumn entries on the stack.  If any
**       of those entries are NULL, jump immediately to brk,
**       which is the loop exit, since no index entry will match
**       if any part of the key is NULL. Pop (nColumn+nExtra) 
**       elements from the stack.
**
**    *  Construct a probe entry from the top nColumn entries in
**       the stack with affinities appropriate for index pIdx. 
**       Only nColumn elements are popped from the stack in this case
**       (by OP_MakeRecord).
**
*/
/* static void buildIndexProbe( */
/*   Vdbe *v, */ 
/*   int nColumn, */ 
/*   int nExtra, */ 
/*   int brk, */ 
/*   Index *pIdx */
/* ){ */
/*   sqlite3VdbeAddOp(v, OP_NotNull, -nColumn, sqlite3VdbeCurrentAddr(v)+3); */
/*   sqlite3VdbeAddOp(v, OP_Pop, nColumn+nExtra, 0); */
/*   sqlite3VdbeAddOp(v, OP_Goto, 0, brk); */
/*   sqlite3VdbeAddOp(v, OP_MakeRecord, nColumn, 0); */
/*   sqlite3IndexAffinityStr(v, pIdx); */
/* } */


/*
** Generate code for a single equality term of the WHERE clause.  An equality
** term can be either X=expr or X IN (...).   pTerm is the term to be 
** coded.
**
** The current value for the constraint is left on the top of the stack.
**
** For a constraint of the form X=expr, the expression is evaluated and its
** result is left on the stack.  For constraints of the form X IN (...)
** this routine sets up a loop that will iterate over all values of X.
*/
/* static void codeEqualityTerm( */
/*   Parse *pParse,      /1* The parsing context *1/ */
/*   WhereTerm *pTerm,   /1* The term of the WHERE clause to be coded *1/ */
/*   int brk,            /1* Jump here to abandon the loop *1/ */
/*   WhereLevel *pLevel  /1* When level of the FROM clause we are working on *1/ */
/* ){ */
/*   Expr *pX = pTerm->pExpr; */
/*   if( pX->op!=TK_IN ){ */
/*     assert( pX->op==TK_EQ ); */
/*     sqlite3ExprCode(pParse, pX->pRight); */
/* #ifndef SQLITE_OMIT_SUBQUERY */
/*   }else{ */
/*     int iTab; */
/*     int *aIn; */
/*     Vdbe *v = pParse->pVdbe; */

/*     sqlite3CodeSubselect(pParse, pX); */
/*     iTab = pX->iTable; */
/*     sqlite3VdbeAddOp(v, OP_Rewind, iTab, brk); */
/*     VdbeComment((v, "# %.*s", pX->span.n, pX->span.z)); */
/*     pLevel->nIn++; */
/*     sqliteReallocOrFree((void**)&pLevel->aInLoop, */
/*                                  sizeof(pLevel->aInLoop[0])*3*pLevel->nIn); */
/*     aIn = pLevel->aInLoop; */
/*     if( aIn ){ */
/*       aIn += pLevel->nIn*3 - 3; */
/*       aIn[0] = OP_Next; */
/*       aIn[1] = iTab; */
/*       aIn[2] = sqlite3VdbeAddOp(v, OP_Column, iTab, 0); */
/*     }else{ */
/*       pLevel->nIn = 0; */
/*     } */
/* #endif */
/*   } */
/*   disableTerm(pLevel, pTerm); */
/* } */

/*
** Generate code that will evaluate all == and IN constraints for an
** index.  The values for all constraints are left on the stack.
**
** For example, consider table t1(a,b,c,d,e,f) with index i1(a,b,c).
** Suppose the WHERE clause is this:  a==5 AND b IN (1,2,3) AND c>5 AND c<10
** The index has as many as three equality constraints, but in this
** example, the third "c" value is an inequality.  So only two 
** constraints are coded.  This routine will generate code to evaluate
** a==5 and b IN (1,2,3).  The current values for a and b will be left
** on the stack - a is the deepest and b the shallowest.
**
** In the example above nEq==2.  But this subroutine works for any value
** of nEq including 0.  If nEq==0, this routine is nearly a no-op.
** The only thing it does is allocate the pLevel->iMem memory cell.
**
** This routine always allocates at least one memory cell and puts
** the address of that memory cell in pLevel->iMem.  The code that
** calls this routine will use pLevel->iMem to store the termination
** key value of the loop.  If one or more IN operators appear, then
** this routine allocates an additional nEq memory cells for internal
** use.
*/
/* static void codeAllEqualityTerms( */
/*   Parse *pParse,        /1* Parsing context *1/ */
/*   WhereLevel *pLevel,   /1* Which nested loop of the FROM we are coding *1/ */
/*   WhereClause *pWC,     /1* The WHERE clause *1/ */
/*   Bitmask notReady,     /1* Which parts of FROM have not yet been coded *1/ */
/*   int brk               /1* Jump here to end the loop *1/ */
/* ){ */
/*   int nEq = pLevel->nEq;        /1* The number of == or IN constraints to code *1/ */
/*   int termsInMem = 0;           /1* If true, store value in mem[] cells *1/ */
/*   Vdbe *v = pParse->pVdbe;      /1* The virtual machine under construction *1/ */
/*   Index *pIdx = pLevel->pIdx;   /1* The index being used for this loop *1/ */
/*   int iCur = pLevel->iTabCur;   /1* The cursor of the table *1/ */
/*   WhereTerm *pTerm;             /1* A single constraint term *1/ */
/*   int j;                        /1* Loop counter *1/ */

/*   /1* Figure out how many memory cells we will need then allocate them. */
/*   ** We always need at least one used to store the loop terminator */
/*   ** value.  If there are IN operators we'll need one for each == or */
/*   ** IN constraint. */
/*   *1/ */
/*   pLevel->iMem = pParse->nMem++; */
/*   if( pLevel->flags & WHERE_COLUMN_IN ){ */
/*     pParse->nMem += pLevel->nEq; */
/*     termsInMem = 1; */
/*   } */

/*   /1* Evaluate the equality constraints */
/*   *1/ */
/*   for(j=0; j<pIdx->nColumn; j++){ */
/*     int k = pIdx->aiColumn[j]; */
/*     pTerm = findTerm(pWC, iCur, k, notReady, WO_EQ|WO_IN, pIdx); */
/*     if( pTerm==0 ) break; */
/*     assert( (pTerm->flags & TERM_CODED)==0 ); */
/*     codeEqualityTerm(pParse, pTerm, brk, pLevel); */
/*     if( termsInMem ){ */
/*       sqlite3VdbeAddOp(v, OP_MemStore, pLevel->iMem+j+1, 1); */
/*     } */
/*   } */
/*   assert( j==nEq ); */

/*   /1* Make sure all the constraint values are on the top of the stack */
/*   *1/ */
/*   if( termsInMem ){ */
/*     for(j=0; j<nEq; j++){ */
/*       sqlite3VdbeAddOp(v, OP_MemLoad, pLevel->iMem+j+1, 0); */
/*     } */
/*   } */
/* } */

#if defined(SQLITE_TEST)
/*
** The following variable holds a text description of query plan generated
** by the most recent call to sqlite3WhereBegin().  Each call to WhereBegin
** overwrites the previous.  This information is used for testing and
** analysis only.
*/
char sqlite3_query_plan[BMS*2*40];  /* Text of the join */
static int nQPlan = 0;              /* Next free slow in _query_plan[] */

#endif /* SQLITE_TEST */



/*
** Generate the beginning of the loop used for WHERE clause processing.
** The return value is a pointer to an opaque structure that contains
** information needed to terminate the loop.  Later, the calling routine
** should invoke sqlite3WhereEnd() with the return value of this function
** in order to complete the WHERE clause processing.
**
** If an error occurs, this routine returns NULL.
**
** The basic idea is to do a nested loop, one loop for each table in
** the FROM clause of a select.  (INSERT and UPDATE statements are the
** same as a SELECT with only a single table in the FROM clause.)  For
** example, if the SQL is this:
**
**       SELECT * FROM t1, t2, t3 WHERE ...;
**
** Then the code generated is conceptually like the following:
**
**      foreach row1 in t1 do       \    Code generated
**        foreach row2 in t2 do      |-- by sqlite3WhereBegin()
**          foreach row3 in t3 do   /
**            ...
**          end                     \    Code generated
**        end                        |-- by sqlite3WhereEnd()
**      end                         /
**
** Note that the loops might not be nested in the order in which they
** appear in the FROM clause if a different order is better able to make
** use of indices.  Note also that when the IN operator appears in
** the WHERE clause, it might result in additional nested loops for
** scanning through all values on the right-hand side of the IN.
**
** There are Btree cursors associated with each table.  t1 uses cursor
** number pTabList->a[0].iCursor.  t2 uses the cursor pTabList->a[1].iCursor.
** And so forth.  This routine generates code to open those VDBE cursors
** and sqlite3WhereEnd() generates the code to close them.
**
** The code that sqlite3WhereBegin() generates leaves the cursors named
** in pTabList pointing at their appropriate entries.  The [...] code
** can use OP_Column and OP_Rowid opcodes on these cursors to extract
** data from the various tables of the loop.
**
** If the WHERE clause is empty, the foreach loops must each scan their
** entire tables.  Thus a three-way join is an O(N^3) operation.  But if
** the tables have indices and there are terms in the WHERE clause that
** refer to those indices, a complete table scan can be avoided and the
** code will run much faster.  Most of the work of this routine is checking
** to see if there are indices that can be used to speed up the loop.
**
** Terms of the WHERE clause are also used to limit which rows actually
** make it to the "..." in the middle of the loop.  After each "foreach",
** terms of the WHERE clause that use only terms in that loop and outer
** loops are evaluated and if false a jump is made around all subsequent
** inner loops (or around the "..." if the test occurs within the inner-
** most loop)
**
** OUTER JOINS
**
** An outer join of tables t1 and t2 is conceptally coded as follows:
**
**    foreach row1 in t1 do
**      flag = 0
**      foreach row2 in t2 do
**        start:
**          ...
**          flag = 1
**      end
**      if flag==0 then
**        move the row2 cursor to a null row
**        goto start
**      fi
**    end
**
** ORDER BY CLAUSE PROCESSING
**
** *ppOrderBy is a pointer to the ORDER BY clause of a SELECT statement,
** if there is one.  If there is no ORDER BY clause or if this routine
** is called from an UPDATE or DELETE statement, then ppOrderBy is NULL.
**
** If an index can be used so that the natural output order of the table
** scan is correct for the ORDER BY clause, then that index is used and
** *ppOrderBy is set to NULL.  This is an optimization that prevents an
** unnecessary sort of the result set if an index appropriate for the
** ORDER BY clause already exists.
**
** If the where clause loops cannot be arranged to provide the correct
** output order, then the *ppOrderBy is unchanged.
*/
/* WhereInfo *sqlite3WhereBegin( */
/*   Parse *pParse,        /1* The parser context *1/ */
/*   SrcList *pTabList,    /1* A list of all tables to be scanned *1/ */
/*   Expr *pWhere,         /1* The WHERE clause *1/ */
/*   ExprList **ppOrderBy  /1* An ORDER BY clause, or NULL *1/ */
/* ){ */
/*   int i;                     /1* Loop counter *1/ */
/*   WhereInfo *pWInfo;         /1* Will become the return value of this function *1/ */
/*   Vdbe *v = pParse->pVdbe;   /1* The virtual database engine *1/ */
/*   int brk, cont = 0;         /1* Addresses used during code generation *1/ */
/*   Bitmask notReady;          /1* Cursors that are not yet positioned *1/ */
/*   WhereTerm *pTerm;          /1* A single term in the WHERE clause *1/ */
/*   ExprMaskSet maskSet;       /1* The expression mask set *1/ */
/*   WhereClause wc;            /1* The WHERE clause is divided into these terms *1/ */
/*   struct SrcList_item *pTabItem;  /1* A single entry from pTabList *1/ */
/*   WhereLevel *pLevel;             /1* A single level in the pWInfo list *1/ */
/*   int iFrom;                      /1* First unused FROM clause element *1/ */
/*   int andFlags;              /1* AND-ed combination of all wc.a[].flags *1/ */

/*   /1* The number of tables in the FROM clause is limited by the number of */
/*   ** bits in a Bitmask */ 
/*   *1/ */
/*   if( pTabList->nSrc>BMS ){ */
/*     sqlite3ErrorMsg(pParse, "at most %d tables in a join", BMS); */
/*     return 0; */
/*   } */

/*   /1* Split the WHERE clause into separate subexpressions where each */
/*   ** subexpression is separated by an AND operator. */
/*   *1/ */
/*   initMaskSet(&maskSet); */
/*   whereClauseInit(&wc, pParse); */
/*   whereSplit(&wc, pWhere, TK_AND); */
    
/*   /1* Allocate and initialize the WhereInfo structure that will become the */
/*   ** return value. */
/*   *1/ */
/*   pWInfo = sqliteMalloc( sizeof(WhereInfo) + pTabList->nSrc*sizeof(WhereLevel)); */
/*   if( sqlite3MallocFailed() ){ */
/*     goto whereBeginNoMem; */
/*   } */
/*   pWInfo->pParse = pParse; */
/*   pWInfo->pTabList = pTabList; */
/*   pWInfo->iBreak = sqlite3VdbeMakeLabel(v); */

/*   /1* Special case: a WHERE clause that is constant.  Evaluate the */
/*   ** expression and either jump over all of the code or fall thru. */
/*   *1/ */
/*   if( pWhere && (pTabList->nSrc==0 || sqlite3ExprIsConstant(pWhere)) ){ */
/*     sqlite3ExprIfFalse(pParse, pWhere, pWInfo->iBreak, 1); */
/*     pWhere = 0; */
/*   } */

/*   /1* Analyze all of the subexpressions.  Note that exprAnalyze() might */
/*   ** add new virtual terms onto the end of the WHERE clause.  We do not */
/*   ** want to analyze these virtual terms, so start analyzing at the end */
/*   ** and work forward so that the added virtual terms are never processed. */
/*   *1/ */
/*   for(i=0; i<pTabList->nSrc; i++){ */
/*     createMask(&maskSet, pTabList->a[i].iCursor); */
/*   } */
/*   exprAnalyzeAll(pTabList, &maskSet, &wc); */
/*   if( sqlite3MallocFailed() ){ */
/*     goto whereBeginNoMem; */
/*   } */

/*   /1* Chose the best index to use for each table in the FROM clause. */
/*   ** */
/*   ** This loop fills in the following fields: */
/*   ** */
/*   **   pWInfo->a[].pIdx      The index to use for this level of the loop. */
/*   **   pWInfo->a[].flags     WHERE_xxx flags associated with pIdx */
/*   **   pWInfo->a[].nEq       The number of == and IN constraints */
/*   **   pWInfo->a[].iFrom     When term of the FROM clause is being coded */
/*   **   pWInfo->a[].iTabCur   The VDBE cursor for the database table */
/*   **   pWInfo->a[].iIdxCur   The VDBE cursor for the index */
/*   ** */
/*   ** This loop also figures out the nesting order of tables in the FROM */
/*   ** clause. */
/*   *1/ */
/*   notReady = ~(Bitmask)0; */
/*   pTabItem = pTabList->a; */
/*   pLevel = pWInfo->a; */
/*   andFlags = ~0; */
/*   TRACE(("*** Optimizer Start ***\n")); */
/*   for(i=iFrom=0, pLevel=pWInfo->a; i<pTabList->nSrc; i++, pLevel++){ */
/*     Index *pIdx;                /1* Index for FROM table at pTabItem *1/ */
/*     int flags;                  /1* Flags asssociated with pIdx *1/ */
/*     int nEq;                    /1* Number of == or IN constraints *1/ */
/*     double cost;                /1* The cost for pIdx *1/ */
/*     int j;                      /1* For looping over FROM tables *1/ */
/*     Index *pBest = 0;           /1* The best index seen so far *1/ */
/*     int bestFlags = 0;          /1* Flags associated with pBest *1/ */
/*     int bestNEq = 0;            /1* nEq associated with pBest *1/ */
/*     double lowestCost;          /1* Cost of the pBest *1/ */
/*     int bestJ = 0;              /1* The value of j *1/ */
/*     Bitmask m;                  /1* Bitmask value for j or bestJ *1/ */
/*     int once = 0;               /1* True when first table is seen *1/ */

/*     lowestCost = SQLITE_BIG_DBL; */
/*     for(j=iFrom, pTabItem=&pTabList->a[j]; j<pTabList->nSrc; j++, pTabItem++){ */
/*       if( once && */ 
/*           ((pTabItem->jointype & (JT_LEFT|JT_CROSS))!=0 */
/*            || (j>0 && (pTabItem[-1].jointype & (JT_LEFT|JT_CROSS))!=0)) */
/*       ){ */
/*         break; */
/*       } */
/*       m = getMask(&maskSet, pTabItem->iCursor); */
/*       if( (m & notReady)==0 ){ */
/*         if( j==iFrom ) iFrom++; */
/*         continue; */
/*       } */
/*       cost = bestIndex(pParse, &wc, pTabItem, notReady, */
/*                        (i==0 && ppOrderBy) ? *ppOrderBy : 0, */
/*                        &pIdx, &flags, &nEq); */
/*       if( cost<lowestCost ){ */
/*         once = 1; */
/*         lowestCost = cost; */
/*         pBest = pIdx; */
/*         bestFlags = flags; */
/*         bestNEq = nEq; */
/*         bestJ = j; */
/*       } */
/*     } */
/*     TRACE(("*** Optimizer choose table %d for loop %d\n", bestJ, */
/*            pLevel-pWInfo->a)); */
/*     if( (bestFlags & WHERE_ORDERBY)!=0 ){ */
/*       *ppOrderBy = 0; */
/*     } */
/*     andFlags &= bestFlags; */
/*     pLevel->flags = bestFlags; */
/*     pLevel->pIdx = pBest; */
/*     pLevel->nEq = bestNEq; */
/*     pLevel->aInLoop = 0; */
/*     pLevel->nIn = 0; */
/*     if( pBest ){ */
/*       pLevel->iIdxCur = pParse->nTab++; */
/*     }else{ */
/*       pLevel->iIdxCur = -1; */
/*     } */
/*     notReady &= ~getMask(&maskSet, pTabList->a[bestJ].iCursor); */
/*     pLevel->iFrom = bestJ; */
/*   } */
/*   TRACE(("*** Optimizer Finished ***\n")); */

/*   /1* If the total query only selects a single row, then the ORDER BY */
/*   ** clause is irrelevant. */
/*   *1/ */
/*   if( (andFlags & WHERE_UNIQUE)!=0 && ppOrderBy ){ */
/*     *ppOrderBy = 0; */
/*   } */

/*   /1* Open all tables in the pTabList and any indices selected for */
/*   ** searching those tables. */
/*   *1/ */
/*   sqlite3CodeVerifySchema(pParse, -1); /1* Insert the cookie verifier Goto *1/ */
/*   pLevel = pWInfo->a; */
/*   for(i=0, pLevel=pWInfo->a; i<pTabList->nSrc; i++, pLevel++){ */
/*     Table *pTab;     /1* Table to open *1/ */
/*     Index *pIx;      /1* Index used to access pTab (if any) *1/ */
/*     int iDb;         /1* Index of database containing table/index *1/ */
/*     int iIdxCur = pLevel->iIdxCur; */

/* #ifndef SQLITE_OMIT_EXPLAIN */
/*     if( pParse->explain==2 ){ */
/*       char *zMsg; */
/*       struct SrcList_item *pItem = &pTabList->a[pLevel->iFrom]; */
/*       zMsg = sqlite3MPrintf("TABLE %s", pItem->zName); */
/*       if( pItem->zAlias ){ */
/*         zMsg = sqlite3MPrintf("%z AS %s", zMsg, pItem->zAlias); */
/*       } */
/*       if( (pIx = pLevel->pIdx)!=0 ){ */
/*         zMsg = sqlite3MPrintf("%z WITH INDEX %s", zMsg, pIx->zName); */
/*       }else if( pLevel->flags & (WHERE_ROWID_EQ|WHERE_ROWID_RANGE) ){ */
/*         zMsg = sqlite3MPrintf("%z USING PRIMARY KEY", zMsg); */
/*       } */
/*       sqlite3VdbeOp3(v, OP_Explain, i, pLevel->iFrom, zMsg, P3_DYNAMIC); */
/*     } */
/* #endif /1* SQLITE_OMIT_EXPLAIN *1/ */
/*     pTabItem = &pTabList->a[pLevel->iFrom]; */
/*     pTab = pTabItem->pTab; */
/*     iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema); */
/*     if( pTab->isTransient || pTab->pSelect ) continue; */
/*     if( (pLevel->flags & WHERE_IDX_ONLY)==0 ){ */
/*       sqlite3OpenTable(pParse, pTabItem->iCursor, iDb, pTab, OP_OpenRead); */
/*       if( pTab->nCol<(sizeof(Bitmask)*8) ){ */
/*         Bitmask b = pTabItem->colUsed; */
/*         int n = 0; */
/*         for(; b; b=b>>1, n++){} */
/*         sqlite3VdbeChangeP2(v, sqlite3VdbeCurrentAddr(v)-1, n); */
/*         assert( n<=pTab->nCol ); */
/*       } */
/*     }else{ */
/*       sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName); */
/*     } */
/*     pLevel->iTabCur = pTabItem->iCursor; */
/*     if( (pIx = pLevel->pIdx)!=0 ){ */
/*       KeyInfo *pKey = sqlite3IndexKeyinfo(pParse, pIx); */
/*       assert( pIx->pSchema==pTab->pSchema ); */
/*       sqlite3VdbeAddOp(v, OP_Integer, iDb, 0); */
/*       VdbeComment((v, "# %s", pIx->zName)); */
/*       sqlite3VdbeOp3(v, OP_OpenRead, iIdxCur, pIx->tnum, */
/*                      (char*)pKey, P3_KEYINFO_HANDOFF); */
/*     } */
/*     if( (pLevel->flags & WHERE_IDX_ONLY)!=0 ){ */
/*       sqlite3VdbeAddOp(v, OP_SetNumColumns, iIdxCur, pIx->nColumn+1); */
/*     } */
/*     sqlite3CodeVerifySchema(pParse, iDb); */
/*   } */
/*   pWInfo->iTop = sqlite3VdbeCurrentAddr(v); */

/*   /1* Generate the code to do the search.  Each iteration of the for */
/*   ** loop below generates code for a single nested loop of the VM */
/*   ** program. */
/*   *1/ */
/*   notReady = ~(Bitmask)0; */
/*   for(i=0, pLevel=pWInfo->a; i<pTabList->nSrc; i++, pLevel++){ */
/*     int j; */
/*     int iCur = pTabItem->iCursor;  /1* The VDBE cursor for the table *1/ */
/*     Index *pIdx;       /1* The index we will be using *1/ */
/*     int iIdxCur;       /1* The VDBE cursor for the index *1/ */
/*     int omitTable;     /1* True if we use the index only *1/ */
/*     int bRev;          /1* True if we need to scan in reverse order *1/ */

/*     pTabItem = &pTabList->a[pLevel->iFrom]; */
/*     iCur = pTabItem->iCursor; */
/*     pIdx = pLevel->pIdx; */
/*     iIdxCur = pLevel->iIdxCur; */
/*     bRev = (pLevel->flags & WHERE_REVERSE)!=0; */
/*     omitTable = (pLevel->flags & WHERE_IDX_ONLY)!=0; */

/*     /1* Create labels for the "break" and "continue" instructions */
/*     ** for the current loop.  Jump to brk to break out of a loop. */
/*     ** Jump to cont to go immediately to the next iteration of the */
/*     ** loop. */
/*     *1/ */
/*     brk = pLevel->brk = sqlite3VdbeMakeLabel(v); */
/*     cont = pLevel->cont = sqlite3VdbeMakeLabel(v); */

/*     /1* If this is the right table of a LEFT OUTER JOIN, allocate and */
/*     ** initialize a memory cell that records if this table matches any */
/*     ** row of the left table of the join. */
/*     *1/ */
/*     if( pLevel->iFrom>0 && (pTabItem[-1].jointype & JT_LEFT)!=0 ){ */
/*       if( !pParse->nMem ) pParse->nMem++; */
/*       pLevel->iLeftJoin = pParse->nMem++; */
/*       sqlite3VdbeAddOp(v, OP_MemInt, 0, pLevel->iLeftJoin); */
/*       VdbeComment((v, "# init LEFT JOIN no-match flag")); */
/*     } */

/*     if( pLevel->flags & WHERE_ROWID_EQ ){ */
/*       /1* Case 1:  We can directly reference a single row using an */
/*       **          equality comparison against the ROWID field.  Or */
/*       **          we reference multiple rows using a "rowid IN (...)" */
/*       **          construct. */
/*       *1/ */
/*       pTerm = findTerm(&wc, iCur, -1, notReady, WO_EQ|WO_IN, 0); */
/*       assert( pTerm!=0 ); */
/*       assert( pTerm->pExpr!=0 ); */
/*       assert( pTerm->leftCursor==iCur ); */
/*       assert( omitTable==0 ); */
/*       codeEqualityTerm(pParse, pTerm, brk, pLevel); */
/*       sqlite3VdbeAddOp(v, OP_MustBeInt, 1, brk); */
/*       sqlite3VdbeAddOp(v, OP_NotExists, iCur, brk); */
/*       VdbeComment((v, "pk")); */
/*       pLevel->op = OP_Noop; */
/*     }else if( pLevel->flags & WHERE_ROWID_RANGE ){ */
/*       /1* Case 2:  We have an inequality comparison against the ROWID field. */
/*       *1/ */
/*       int testOp = OP_Noop; */
/*       int start; */
/*       WhereTerm *pStart, *pEnd; */

/*       assert( omitTable==0 ); */
/*       pStart = findTerm(&wc, iCur, -1, notReady, WO_GT|WO_GE, 0); */
/*       pEnd = findTerm(&wc, iCur, -1, notReady, WO_LT|WO_LE, 0); */
/*       if( bRev ){ */
/*         pTerm = pStart; */
/*         pStart = pEnd; */
/*         pEnd = pTerm; */
/*       } */
/*       if( pStart ){ */
/*         Expr *pX; */
/*         pX = pStart->pExpr; */
/*         assert( pX!=0 ); */
/*         assert( pStart->leftCursor==iCur ); */
/*         sqlite3ExprCode(pParse, pX->pRight); */
/*         sqlite3VdbeAddOp(v, OP_ForceInt, pX->op==TK_LE || pX->op==TK_GT, brk); */
/*         sqlite3VdbeAddOp(v, bRev ? OP_MoveLt : OP_MoveGe, iCur, brk); */
/*         VdbeComment((v, "pk")); */
/*         disableTerm(pLevel, pStart); */
/*       }else{ */
/*         sqlite3VdbeAddOp(v, bRev ? OP_Last : OP_Rewind, iCur, brk); */
/*       } */
/*       if( pEnd ){ */
/*         Expr *pX; */
/*         pX = pEnd->pExpr; */
/*         assert( pX!=0 ); */
/*         assert( pEnd->leftCursor==iCur ); */
/*         sqlite3ExprCode(pParse, pX->pRight); */
/*         pLevel->iMem = pParse->nMem++; */
/*         sqlite3VdbeAddOp(v, OP_MemStore, pLevel->iMem, 1); */
/*         if( pX->op==TK_LT || pX->op==TK_GT ){ */
/*           testOp = bRev ? OP_Le : OP_Ge; */
/*         }else{ */
/*           testOp = bRev ? OP_Lt : OP_Gt; */
/*         } */
/*         disableTerm(pLevel, pEnd); */
/*       } */
/*       start = sqlite3VdbeCurrentAddr(v); */
/*       pLevel->op = bRev ? OP_Prev : OP_Next; */
/*       pLevel->p1 = iCur; */
/*       pLevel->p2 = start; */
/*       if( testOp!=OP_Noop ){ */
/*         sqlite3VdbeAddOp(v, OP_Rowid, iCur, 0); */
/*         sqlite3VdbeAddOp(v, OP_MemLoad, pLevel->iMem, 0); */
/*         sqlite3VdbeAddOp(v, testOp, SQLITE_AFF_NUMERIC, brk); */
/*       } */
/*     }else if( pLevel->flags & WHERE_COLUMN_RANGE ){ */
/*       /1* Case 3: The WHERE clause term that refers to the right-most */
/*       **         column of the index is an inequality.  For example, if */
/*       **         the index is on (x,y,z) and the WHERE clause is of the */
/*       **         form "x=5 AND y<10" then this case is used.  Only the */
/*       **         right-most column can be an inequality - the rest must */
/*       **         use the "==" and "IN" operators. */
/*       ** */
/*       **         This case is also used when there are no WHERE clause */
/*       **         constraints but an index is selected anyway, in order */
/*       **         to force the output order to conform to an ORDER BY. */
/*       *1/ */
/*       int start; */
/*       int nEq = pLevel->nEq; */
/*       int topEq=0;        /1* True if top limit uses ==. False is strictly < *1/ */
/*       int btmEq=0;        /1* True if btm limit uses ==. False if strictly > *1/ */
/*       int topOp, btmOp;   /1* Operators for the top and bottom search bounds *1/ */
/*       int testOp; */
/*       int nNotNull;       /1* Number of rows of index that must be non-NULL *1/ */
/*       int topLimit = (pLevel->flags & WHERE_TOP_LIMIT)!=0; */
/*       int btmLimit = (pLevel->flags & WHERE_BTM_LIMIT)!=0; */

/*       /1* Generate code to evaluate all constraint terms using == or IN */
/*       ** and level the values of those terms on the stack. */
/*       *1/ */
/*       codeAllEqualityTerms(pParse, pLevel, &wc, notReady, brk); */

/*       /1* Duplicate the equality term values because they will all be */
/*       ** used twice: once to make the termination key and once to make the */
/*       ** start key. */
/*       *1/ */
/*       for(j=0; j<nEq; j++){ */
/*         sqlite3VdbeAddOp(v, OP_Dup, nEq-1, 0); */
/*       } */

/*       /1* Figure out what comparison operators to use for top and bottom */ 
/*       ** search bounds. For an ascending index, the bottom bound is a > or >= */
/*       ** operator and the top bound is a < or <= operator.  For a descending */
/*       ** index the operators are reversed. */
/*       *1/ */
/*       nNotNull = nEq + topLimit; */
/*       if( pIdx->aSortOrder[nEq]==SQLITE_SO_ASC ){ */
/*         topOp = WO_LT|WO_LE; */
/*         btmOp = WO_GT|WO_GE; */
/*       }else{ */
/*         topOp = WO_GT|WO_GE; */
/*         btmOp = WO_LT|WO_LE; */
/*         SWAP(int, topLimit, btmLimit); */
/*       } */

/*       /1* Generate the termination key.  This is the key value that */
/*       ** will end the search.  There is no termination key if there */
/*       ** are no equality terms and no "X<..." term. */
/*       ** */
/*       ** 2002-Dec-04: On a reverse-order scan, the so-called "termination" */
/*       ** key computed here really ends up being the start key. */
/*       *1/ */
/*       if( topLimit ){ */
/*         Expr *pX; */
/*         int k = pIdx->aiColumn[j]; */
/*         pTerm = findTerm(&wc, iCur, k, notReady, topOp, pIdx); */
/*         assert( pTerm!=0 ); */
/*         pX = pTerm->pExpr; */
/*         assert( (pTerm->flags & TERM_CODED)==0 ); */
/*         sqlite3ExprCode(pParse, pX->pRight); */
/*         topEq = pTerm->eOperator & (WO_LE|WO_GE); */
/*         disableTerm(pLevel, pTerm); */
/*         testOp = OP_IdxGE; */
/*       }else{ */
/*         testOp = nEq>0 ? OP_IdxGE : OP_Noop; */
/*         topEq = 1; */
/*       } */
/*       if( testOp!=OP_Noop ){ */
/*         int nCol = nEq + topLimit; */
/*         pLevel->iMem = pParse->nMem++; */
/*         buildIndexProbe(v, nCol, nEq, brk, pIdx); */
/*         if( bRev ){ */
/*           int op = topEq ? OP_MoveLe : OP_MoveLt; */
/*           sqlite3VdbeAddOp(v, op, iIdxCur, brk); */
/*         }else{ */
/*           sqlite3VdbeAddOp(v, OP_MemStore, pLevel->iMem, 1); */
/*         } */
/*       }else if( bRev ){ */
/*         sqlite3VdbeAddOp(v, OP_Last, iIdxCur, brk); */
/*       } */

/*       /1* Generate the start key.  This is the key that defines the lower */
/*       ** bound on the search.  There is no start key if there are no */
/*       ** equality terms and if there is no "X>..." term.  In */
/*       ** that case, generate a "Rewind" instruction in place of the */
/*       ** start key search. */
/*       ** */
/*       ** 2002-Dec-04: In the case of a reverse-order search, the so-called */
/*       ** "start" key really ends up being used as the termination key. */
/*       *1/ */
/*       if( btmLimit ){ */
/*         Expr *pX; */
/*         int k = pIdx->aiColumn[j]; */
/*         pTerm = findTerm(&wc, iCur, k, notReady, btmOp, pIdx); */
/*         assert( pTerm!=0 ); */
/*         pX = pTerm->pExpr; */
/*         assert( (pTerm->flags & TERM_CODED)==0 ); */
/*         sqlite3ExprCode(pParse, pX->pRight); */
/*         btmEq = pTerm->eOperator & (WO_LE|WO_GE); */
/*         disableTerm(pLevel, pTerm); */
/*       }else{ */
/*         btmEq = 1; */
/*       } */
/*       if( nEq>0 || btmLimit ){ */
/*         int nCol = nEq + btmLimit; */
/*         buildIndexProbe(v, nCol, 0, brk, pIdx); */
/*         if( bRev ){ */
/*           pLevel->iMem = pParse->nMem++; */
/*           sqlite3VdbeAddOp(v, OP_MemStore, pLevel->iMem, 1); */
/*           testOp = OP_IdxLT; */
/*         }else{ */
/*           int op = btmEq ? OP_MoveGe : OP_MoveGt; */
/*           sqlite3VdbeAddOp(v, op, iIdxCur, brk); */
/*         } */
/*       }else if( bRev ){ */
/*         testOp = OP_Noop; */
/*       }else{ */
/*         sqlite3VdbeAddOp(v, OP_Rewind, iIdxCur, brk); */
/*       } */

/*       /1* Generate the the top of the loop.  If there is a termination */
/*       ** key we have to test for that key and abort at the top of the */
/*       ** loop. */
/*       *1/ */
/*       start = sqlite3VdbeCurrentAddr(v); */
/*       if( testOp!=OP_Noop ){ */
/*         sqlite3VdbeAddOp(v, OP_MemLoad, pLevel->iMem, 0); */
/*         sqlite3VdbeAddOp(v, testOp, iIdxCur, brk); */
/*         if( (topEq && !bRev) || (!btmEq && bRev) ){ */
/*           sqlite3VdbeChangeP3(v, -1, "+", P3_STATIC); */
/*         } */
/*       } */
/*       sqlite3VdbeAddOp(v, OP_RowKey, iIdxCur, 0); */
/*       sqlite3VdbeAddOp(v, OP_IdxIsNull, nNotNull, cont); */
/*       if( !omitTable ){ */
/*         sqlite3VdbeAddOp(v, OP_IdxRowid, iIdxCur, 0); */
/*         sqlite3VdbeAddOp(v, OP_MoveGe, iCur, 0); */
/*       } */

/*       /1* Record the instruction used to terminate the loop. */
/*       *1/ */
/*       pLevel->op = bRev ? OP_Prev : OP_Next; */
/*       pLevel->p1 = iIdxCur; */
/*       pLevel->p2 = start; */
/*     }else if( pLevel->flags & WHERE_COLUMN_EQ ){ */
/*       /1* Case 4:  There is an index and all terms of the WHERE clause that */
/*       **          refer to the index using the "==" or "IN" operators. */
/*       *1/ */
/*       int start; */
/*       int nEq = pLevel->nEq; */

/*       /1* Generate code to evaluate all constraint terms using == or IN */
/*       ** and leave the values of those terms on the stack. */
/*       *1/ */
/*       codeAllEqualityTerms(pParse, pLevel, &wc, notReady, brk); */

/*       /1* Generate a single key that will be used to both start and terminate */
/*       ** the search */
/*       *1/ */
/*       buildIndexProbe(v, nEq, 0, brk, pIdx); */
/*       sqlite3VdbeAddOp(v, OP_MemStore, pLevel->iMem, 0); */

/*       /1* Generate code (1) to move to the first matching element of the table. */
/*       ** Then generate code (2) that jumps to "brk" after the cursor is past */
/*       ** the last matching element of the table.  The code (1) is executed */
/*       ** once to initialize the search, the code (2) is executed before each */
/*       ** iteration of the scan to see if the scan has finished. *1/ */
/*       if( bRev ){ */
/*         /1* Scan in reverse order *1/ */
/*         sqlite3VdbeAddOp(v, OP_MoveLe, iIdxCur, brk); */
/*         start = sqlite3VdbeAddOp(v, OP_MemLoad, pLevel->iMem, 0); */
/*         sqlite3VdbeAddOp(v, OP_IdxLT, iIdxCur, brk); */
/*         pLevel->op = OP_Prev; */
/*       }else{ */
/*         /1* Scan in the forward order *1/ */
/*         sqlite3VdbeAddOp(v, OP_MoveGe, iIdxCur, brk); */
/*         start = sqlite3VdbeAddOp(v, OP_MemLoad, pLevel->iMem, 0); */
/*         sqlite3VdbeOp3(v, OP_IdxGE, iIdxCur, brk, "+", P3_STATIC); */
/*         pLevel->op = OP_Next; */
/*       } */
/*       sqlite3VdbeAddOp(v, OP_RowKey, iIdxCur, 0); */
/*       sqlite3VdbeAddOp(v, OP_IdxIsNull, nEq, cont); */
/*       if( !omitTable ){ */
/*         sqlite3VdbeAddOp(v, OP_IdxRowid, iIdxCur, 0); */
/*         sqlite3VdbeAddOp(v, OP_MoveGe, iCur, 0); */
/*       } */
/*       pLevel->p1 = iIdxCur; */
/*       pLevel->p2 = start; */
/*     }else{ */
/*       /1* Case 5:  There is no usable index.  We must do a complete */
/*       **          scan of the entire table. */
/*       *1/ */
/*       assert( omitTable==0 ); */
/*       assert( bRev==0 ); */
/*       pLevel->op = OP_Next; */
/*       pLevel->p1 = iCur; */
/*       pLevel->p2 = 1 + sqlite3VdbeAddOp(v, OP_Rewind, iCur, brk); */
/*     } */
/*     notReady &= ~getMask(&maskSet, iCur); */

/*     /1* Insert code to test every subexpression that can be completely */
/*     ** computed using the current set of tables. */
/*     *1/ */
/*     for(pTerm=wc.a, j=wc.nTerm; j>0; j--, pTerm++){ */
/*       Expr *pE; */
/*       if( pTerm->flags & (TERM_VIRTUAL|TERM_CODED) ) continue; */
/*       if( (pTerm->prereqAll & notReady)!=0 ) continue; */
/*       pE = pTerm->pExpr; */
/*       assert( pE!=0 ); */
/*       if( pLevel->iLeftJoin && !ExprHasProperty(pE, EP_FromJoin) ){ */
/*         continue; */
/*       } */
/*       sqlite3ExprIfFalse(pParse, pE, cont, 1); */
/*       pTerm->flags |= TERM_CODED; */
/*     } */

/*     /1* For a LEFT OUTER JOIN, generate code that will record the fact that */
/*     ** at least one row of the right table has matched the left table. */  
/*     *1/ */
/*     if( pLevel->iLeftJoin ){ */
/*       pLevel->top = sqlite3VdbeCurrentAddr(v); */
/*       sqlite3VdbeAddOp(v, OP_MemInt, 1, pLevel->iLeftJoin); */
/*       VdbeComment((v, "# record LEFT JOIN hit")); */
/*       for(pTerm=wc.a, j=0; j<wc.nTerm; j++, pTerm++){ */
/*         if( pTerm->flags & (TERM_VIRTUAL|TERM_CODED) ) continue; */
/*         if( (pTerm->prereqAll & notReady)!=0 ) continue; */
/*         assert( pTerm->pExpr ); */
/*         sqlite3ExprIfFalse(pParse, pTerm->pExpr, cont, 1); */
/*         pTerm->flags |= TERM_CODED; */
/*       } */
/*     } */
/*   } */

/* #ifdef SQLITE_TEST  /1* For testing and debugging use only *1/ */
/*   /1* Record in the query plan information about the current table */
/*   ** and the index used to access it (if any).  If the table itself */
/*   ** is not used, its name is just '{}'.  If no index is used */
/*   ** the index is listed as "{}".  If the primary key is used the */
/*   ** index name is '*'. */
/*   *1/ */
/*   for(i=0; i<pTabList->nSrc; i++){ */
/*     char *z; */
/*     int n; */
/*     pLevel = &pWInfo->a[i]; */
/*     pTabItem = &pTabList->a[pLevel->iFrom]; */
/*     z = pTabItem->zAlias; */
/*     if( z==0 ) z = pTabItem->pTab->zName; */
/*     n = strlen(z); */
/*     if( n+nQPlan < sizeof(sqlite3_query_plan)-10 ){ */
/*       if( pLevel->flags & WHERE_IDX_ONLY ){ */
/*         strcpy(&sqlite3_query_plan[nQPlan], "{}"); */
/*         nQPlan += 2; */
/*       }else{ */
/*         strcpy(&sqlite3_query_plan[nQPlan], z); */
/*         nQPlan += n; */
/*       } */
/*       sqlite3_query_plan[nQPlan++] = ' '; */
/*     } */
/*     if( pLevel->flags & (WHERE_ROWID_EQ|WHERE_ROWID_RANGE) ){ */
/*       strcpy(&sqlite3_query_plan[nQPlan], "* "); */
/*       nQPlan += 2; */
/*     }else if( pLevel->pIdx==0 ){ */
/*       strcpy(&sqlite3_query_plan[nQPlan], "{} "); */
/*       nQPlan += 3; */
/*     }else{ */
/*       n = strlen(pLevel->pIdx->zName); */
/*       if( n+nQPlan < sizeof(sqlite3_query_plan)-2 ){ */
/*         strcpy(&sqlite3_query_plan[nQPlan], pLevel->pIdx->zName); */
/*         nQPlan += n; */
/*         sqlite3_query_plan[nQPlan++] = ' '; */
/*       } */
/*     } */
/*   } */
/*   while( nQPlan>0 && sqlite3_query_plan[nQPlan-1]==' ' ){ */
/*     sqlite3_query_plan[--nQPlan] = 0; */
/*   } */
/*   sqlite3_query_plan[nQPlan] = 0; */
/*   nQPlan = 0; */
/* #endif /1* SQLITE_TEST // Testing and debugging use only *1/ */

/*   /1* Record the continuation address in the WhereInfo structure.  Then */
/*   ** clean up and return. */
/*   *1/ */
/*   pWInfo->iContinue = cont; */
/*   whereClauseClear(&wc); */
/*   return pWInfo; */

/*   /1* Jump here if malloc fails *1/ */
/* whereBeginNoMem: */
/*   whereClauseClear(&wc); */
/*   sqliteFree(pWInfo); */
/*   return 0; */
/* } */

/*
** Generate the end of the WHERE loop.  See comments on 
** sqlite3WhereBegin() for additional information.
*/
/* void sqlite3WhereEnd(WhereInfo *pWInfo){ */
/*   Vdbe *v = pWInfo->pParse->pVdbe; */
/*   int i; */
/*   WhereLevel *pLevel; */
/*   SrcList *pTabList = pWInfo->pTabList; */

/*   /1* Generate loop termination code. */
/*   *1/ */
/*   for(i=pTabList->nSrc-1; i>=0; i--){ */
/*     pLevel = &pWInfo->a[i]; */
/*     sqlite3VdbeResolveLabel(v, pLevel->cont); */
/*     if( pLevel->op!=OP_Noop ){ */
/*       sqlite3VdbeAddOp(v, pLevel->op, pLevel->p1, pLevel->p2); */
/*     } */
/*     sqlite3VdbeResolveLabel(v, pLevel->brk); */
/*     if( pLevel->nIn ){ */
/*       int *a; */
/*       int j; */
/*       for(j=pLevel->nIn, a=&pLevel->aInLoop[j*3-3]; j>0; j--, a-=3){ */
/*         sqlite3VdbeAddOp(v, a[0], a[1], a[2]); */
/*       } */
/*       sqliteFree(pLevel->aInLoop); */
/*     } */
/*     if( pLevel->iLeftJoin ){ */
/*       int addr; */
/*       addr = sqlite3VdbeAddOp(v, OP_IfMemPos, pLevel->iLeftJoin, 0); */
/*       sqlite3VdbeAddOp(v, OP_NullRow, pTabList->a[i].iCursor, 0); */
/*       if( pLevel->iIdxCur>=0 ){ */
/*         sqlite3VdbeAddOp(v, OP_NullRow, pLevel->iIdxCur, 0); */
/*       } */
/*       sqlite3VdbeAddOp(v, OP_Goto, 0, pLevel->top); */
/*       sqlite3VdbeJumpHere(v, addr); */
/*     } */
/*   } */

/*   /1* The "break" point is here, just past the end of the outer loop. */
/*   ** Set it. */
/*   *1/ */
/*   sqlite3VdbeResolveLabel(v, pWInfo->iBreak); */

/*   /1* Close all of the cursors that were opened by sqlite3WhereBegin. */
/*   *1/ */
/*   for(i=0, pLevel=pWInfo->a; i<pTabList->nSrc; i++, pLevel++){ */
/*     struct SrcList_item *pTabItem = &pTabList->a[pLevel->iFrom]; */
/*     Table *pTab = pTabItem->pTab; */
/*     assert( pTab!=0 ); */
/*     if( pTab->isTransient || pTab->pSelect ) continue; */
/*     if( (pLevel->flags & WHERE_IDX_ONLY)==0 ){ */
/*       sqlite3VdbeAddOp(v, OP_Close, pTabItem->iCursor, 0); */
/*     } */
/*     if( pLevel->pIdx!=0 ){ */
/*       sqlite3VdbeAddOp(v, OP_Close, pLevel->iIdxCur, 0); */
/*     } */

/*     /1* Make cursor substitutions for cases where we want to use */
/*     ** just the index and never reference the table. */
/*     ** */ 
/*     ** Calls to the code generator in between sqlite3WhereBegin and */
/*     ** sqlite3WhereEnd will have created code that references the table */
/*     ** directly.  This loop scans all that code looking for opcodes */
/*     ** that reference the table and converts them into opcodes that */
/*     ** reference the index. */
/*     *1/ */
/*     if( pLevel->flags & WHERE_IDX_ONLY ){ */
/*       int k, j, last; */
/*       VdbeOp *pOp; */
/*       Index *pIdx = pLevel->pIdx; */

/*       assert( pIdx!=0 ); */
/*       pOp = sqlite3VdbeGetOp(v, pWInfo->iTop); */
/*       last = sqlite3VdbeCurrentAddr(v); */
/*       for(k=pWInfo->iTop; k<last; k++, pOp++){ */
/*         if( pOp->p1!=pLevel->iTabCur ) continue; */
/*         if( pOp->opcode==OP_Column ){ */
/*           pOp->p1 = pLevel->iIdxCur; */
/*           for(j=0; j<pIdx->nColumn; j++){ */
/*             if( pOp->p2==pIdx->aiColumn[j] ){ */
/*               pOp->p2 = j; */
/*               break; */
/*             } */
/*           } */
/*         }else if( pOp->opcode==OP_Rowid ){ */
/*           pOp->p1 = pLevel->iIdxCur; */
/*           pOp->opcode = OP_IdxRowid; */
/*         }else if( pOp->opcode==OP_NullRow ){ */
/*           pOp->opcode = OP_Noop; */
/*         } */
/*       } */
/*     } */
/*   } */

/*   /1* Final cleanup */
/*   *1/ */
/*   sqliteFree(pWInfo); */
/*   return; */
/* } */
/*************************************file from tokenize.c************************************/
/*
** The charMap() macro maps alphabetic characters into their
** lower-case ASCII equivalent.  On ASCII machines, this is just
** an upper-to-lower case map.  On EBCDIC machines we also need
** to adjust the encoding.  Only alphabetic characters and underscores
** need to be translated.
*/
#ifdef SQLITE_ASCII
# define charMap(X) sqlite3UpperToLower[(unsigned char)X]
#endif
#ifdef SQLITE_EBCDIC
# define charMap(X) ebcdicToAscii[(unsigned char)X]
const unsigned char ebcdicToAscii[] = {
/* 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 0x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 1x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 2x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 3x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 4x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 5x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 95,  0,  0,  /* 6x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 7x */
   0, 97, 98, 99,100,101,102,103,104,105,  0,  0,  0,  0,  0,  0,  /* 8x */
   0,106,107,108,109,110,111,112,113,114,  0,  0,  0,  0,  0,  0,  /* 9x */
   0,  0,115,116,117,118,119,120,121,122,  0,  0,  0,  0,  0,  0,  /* Ax */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* Bx */
   0, 97, 98, 99,100,101,102,103,104,105,  0,  0,  0,  0,  0,  0,  /* Cx */
   0,106,107,108,109,110,111,112,113,114,  0,  0,  0,  0,  0,  0,  /* Dx */
   0,  0,115,116,117,118,119,120,121,122,  0,  0,  0,  0,  0,  0,  /* Ex */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* Fx */
};
#endif

/*
** If X is a character that can be used in an identifier then
** IdChar(X) will be true.  Otherwise it is false.
**
** For ASCII, any character with the high-order bit set is
** allowed in an identifier.  For 7-bit characters, 
** sqlite3IsIdChar[X] must be 1.
**
** For EBCDIC, the rules are more complex but have the same
** end result.
**
** Ticket #1066.  the SQL standard does not allow '$' in the
** middle of identfiers.  But many SQL implementations do. 
** SQLite will allow '$' in identifiers for compatibility.
** But the feature is undocumented.
*/
#ifdef SQLITE_ASCII
const char sqlite3IsIdChar[] = {
/* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF */
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 2x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  /* 3x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 4x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,  /* 5x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 6x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  /* 7x */
};
#define IdChar(C)  (((c=C)&0x80)!=0 || (c>0x1f && sqlite3IsIdChar[c-0x20]))
#endif
#ifdef SQLITE_EBCDIC
const char sqlite3IsIdChar[] = {
/* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  /* 4x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0,  /* 5x */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0,  /* 6x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,  /* 7x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0,  /* 8x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0,  /* 9x */
    1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0,  /* Ax */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* Bx */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  /* Cx */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  /* Dx */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  /* Ex */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0,  /* Fx */
};
#define IdChar(C)  (((c=C)>=0x42 && sqlite3IsIdChar[c-0x40]))
#endif


/*
** Return the length of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
static int getToken(const unsigned char *z, int *tokenType){
  int i, c;
  switch( *z ){
    case ' ': case '\t': case '\n': case '\f': case '\r': {
      for(i=1; isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      return i;
    }
    case '-': {
      if( z[1]=='-' ){
        for(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        return i;
      }
      *tokenType = TK_MINUS;
      return 1;
    }
    case '(': {
      *tokenType = TK_LP;
      return 1;
    }
    case ')': {
      *tokenType = TK_RP;
      return 1;
    }
    case ';': {
      *tokenType = TK_SEMI;
      return 1;
    }
    case '+': {
      *tokenType = TK_PLUS;
      return 1;
    }
    case '*': {
      *tokenType = TK_STAR;
      return 1;
    }
    case '/': {
      if( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        return 1;
      }
      for(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      if( c ) i++;
      *tokenType = TK_COMMENT;
      return i;
    }
    case '%': {
      *tokenType = TK_REM;
      return 1;
    }
    case '=': {
      *tokenType = TK_EQ;
      return 1 + (z[1]=='=');
    }
    case '<': {
      if( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        return 2;
      }else if( c=='>' ){
        *tokenType = TK_NE;
        return 2;
      }else if( c=='<' ){
        *tokenType = TK_LSHIFT;
        return 2;
      }else{
        *tokenType = TK_LT;
        return 1;
      }
    }
    case '>': {
      if( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        return 2;
      }else if( c=='>' ){
        *tokenType = TK_RSHIFT;
        return 2;
      }else{
        *tokenType = TK_GT;
        return 1;
      }
    }
    case '!': {
      if( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        return 2;
      }else{
        *tokenType = TK_NE;
        return 2;
      }
    }
    case '|': {
      if( z[1]!='|' ){
        *tokenType = TK_BITOR;
        return 1;
      }else{
        *tokenType = TK_CONCAT;
        return 2;
      }
    }
    case ',': {
      *tokenType = TK_COMMA;
      return 1;
    }
    case '&': {
      *tokenType = TK_BITAND;
      return 1;
    }
    case '~': {
      *tokenType = TK_BITNOT;
      return 1;
    }
    case '`':
    case '\'':
    case '"': {
      int delim = z[0];
      for(i=1; (c=z[i])!=0; i++){
        if( c==delim ){
          if( z[i+1]==delim ){
            i++;
          }else{
            break;
          }
        }
      }
      if( c ){
        *tokenType = TK_STRING;
        return i+1;
      }else{
        *tokenType = TK_ILLEGAL;
        return i;
      }
    }
    case '.': {
#ifndef SQLITE_OMIT_FLOATING_POINT
      if( !isdigit(z[1]) )
#endif
      {
        *tokenType = TK_DOT;
        return 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
    }
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': {
      *tokenType = TK_INTEGER;
      for(i=0; isdigit(z[i]); i++){}
#ifndef SQLITE_OMIT_FLOATING_POINT
      if( z[i]=='.' ){
        i++;
        while( isdigit(z[i]) ){ i++; }
        *tokenType = TK_FLOAT;
      }
      if( (z[i]=='e' || z[i]=='E') &&
           ( isdigit(z[i+1]) 
            || ((z[i+1]=='+' || z[i+1]=='-') && isdigit(z[i+2]))
           )
      ){
        i += 2;
        while( isdigit(z[i]) ){ i++; }
        *tokenType = TK_FLOAT;
      }
#endif
      return i;
    }
    case '[': {
      for(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = TK_ID;
      return i;
    }
    case '?': {
      *tokenType = TK_VARIABLE;
      for(i=1; isdigit(z[i]); i++){}
      return i;
    }
    case '#': {
      for(i=1; isdigit(z[i]); i++){}
      if( i>1 ){
        /* Parameters of the form #NNN (where NNN is a number) are used
        ** internally by sqlite3NestedParse.  */
        *tokenType = TK_REGISTER;
        return i;
      }
      /* Fall through into the next case if the '#' is not followed by
      ** a digit. Try to match #AAAA where AAAA is a parameter name. */
    }
#ifndef SQLITE_OMIT_TCL_VARIABLE
    case '$':
#endif
    case '@':
    case ':': { /* For compatibility with MS SQL Server */
      int n = 0;
      if ( z[0] == '@' && z[1] == '@') {
        *tokenType = TK_VARIABLE1;
        i = 2;
      } else {
        *tokenType = TK_VARIABLE;
        i = 1;
      }

      for(; (c=z[i])!=0; i++){
        if( IdChar(c) ){
          n++;
#ifndef SQLITE_OMIT_TCL_VARIABLE
        }else if( c=='(' && n>0 ){
          do{
            i++;
          }while( (c=z[i])!=0 && !isspace(c) && c!=')' );
          if( c==')' ){
            i++;
          }else{
            *tokenType = TK_ILLEGAL;
          }
          break;
        }else if( c==':' && z[i+1]==':' ){
          i++;
#endif
        }         
        else{
          break;
        }
      }
      if( n==0 ) *tokenType = TK_ILLEGAL;
      return i;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    case 'x': case 'X': {
      if( (c=z[1])=='\'' || c=='"' ){
        int delim = c;
        *tokenType = TK_BLOB;
        for(i=2; (c=z[i])!=0; i++){
          if( c==delim ){
            if( i%2 ) *tokenType = TK_ILLEGAL;
            break;
          }
          if( !isxdigit(c) ){
            *tokenType = TK_ILLEGAL;
            return i;
          }
        }
        if( c ) i++;
        return i;
      }
      /* Otherwise fall through to the next case */
    }
#endif
    default: {
      if( !IdChar(*z) ){
        break;
      }
      for(i=1; IdChar(z[i]) || isdigit(z[i]) ; i++){}
      *tokenType = keywordCode((char*)z, i);
      return i;
    }
  }
  *tokenType = TK_ILLEGAL;
  return 1;
}
int sqlite3GetToken(const unsigned char *z, int *tokenType){
  return getToken(z, tokenType);
}

/*
** Run the parser on the given SQL string.  The parser structure is
** passed in.  An SQLITE_ status code is returned.  If an error occurs
** and pzErrMsg!=NULL then an error message might be written into 
** memory obtained from malloc() and *pzErrMsg made to point to that
** error message.  Or maybe not.
*/
int sqlite3RunParser(Parse *pParse, const char *zSql, char **pzErrMsg){
  int nErr = 0;
  int i;
  void *pEngine;
  int tokenType;
  int lastTokenParsed = -1;
 // sqlite3 *db = pParse->db;
  extern void *sqlite3ParserAlloc(void*(*)(size_t));
  extern void sqlite3ParserFree(void*, void(*)(void*));
  extern void sqlite3Parser(void*, int, Token, Parse*);

  //db->flags &= ~SQLITE_Interrupt;
  pParse->flags &= ~SQLITE_Interrupt;
  pParse->rc = SQLITE_OK;
  i = 0;
  pEngine = sqlite3ParserAlloc((void*(*)(size_t))sqlite3MallocX);
  if( pEngine==0 ){
    return SQLITE_NOMEM;
  }
  assert( pParse->sLastToken.dyn==0 );
  assert( pParse->pNewTable==0 );
  assert( pParse->pNewTrigger==0 );
  assert( pParse->nVar==0 );
  assert( pParse->nVarExpr==0 );
  assert( pParse->nVarExprAlloc==0 );
  assert( pParse->apVarExpr==0 );
  pParse->zTail = pParse->zSql = zSql;
  while( !sqlite3MallocFailed() && zSql[i]!=0 ){
    assert( i>=0 );
    pParse->sLastToken.z = (u8*)&zSql[i];
    assert( pParse->sLastToken.dyn==0 );
    pParse->sLastToken.n = getToken((unsigned char*)&zSql[i],&tokenType);
    i += pParse->sLastToken.n;

    if (tokenType != TK_SPACE) { 
        TokenItem tokenItem;
        tokenItem.token = pParse->sLastToken;
        tokenItem.tokenType = tokenType;
        sqlite3TokenArrayAppend(&pParse->tokens, &tokenItem); 
    }
    switch( tokenType ){
      case TK_SPACE:
      case TK_COMMENT: {
        //if( (db->flags & SQLITE_Interrupt)!=0 ){
        if( (pParse->flags & SQLITE_Interrupt)!=0 ){
          pParse->rc = SQLITE_INTERRUPT;
          sqlite3SetString(pzErrMsg, "interrupt", (char*)0);
          goto abort_parse;
        }
        break;
      }
      case TK_ILLEGAL: {
        if( pzErrMsg ){
          sqliteFree(*pzErrMsg);
          *pzErrMsg = sqlite3MPrintf("unrecognized token: \"%T\"",
                          &pParse->sLastToken);
        }
        nErr++;
        goto abort_parse;
      }
      case TK_SEMI: {
        pParse->zTail = &zSql[i];
        /* Fall thru into the default case */
      }
      default: {
        sqlite3Parser(pEngine, tokenType, pParse->sLastToken, pParse);
        lastTokenParsed = tokenType;
        if( pParse->rc!=SQLITE_OK ){
          goto abort_parse;
        }
        break;
      }
    }
  }
abort_parse:
    while(!sqlite3MallocFailed() && zSql[i] != 0) {
        pParse->sLastToken.z = (u8*)&zSql[i];
        assert( pParse->sLastToken.dyn==0 );
        pParse->sLastToken.n = getToken((unsigned char*)&zSql[i],&tokenType);
        i += pParse->sLastToken.n;

        if (tokenType != TK_SPACE) { 
            TokenItem tokenItem;
            tokenItem.token = pParse->sLastToken;
            tokenItem.tokenType = tokenType;
            sqlite3TokenArrayAppend(&pParse->tokens, &tokenItem); 
        }
    }  

  if( zSql[i]==0 && nErr==0 && pParse->rc==SQLITE_OK ){
    if( lastTokenParsed!=TK_SEMI ){
      sqlite3Parser(pEngine, TK_SEMI, pParse->sLastToken, pParse);
      pParse->zTail = &zSql[i];
    }
    sqlite3Parser(pEngine, 0, pParse->sLastToken, pParse);
  }
  sqlite3ParserFree(pEngine, sqlite3FreeX);
  if( sqlite3MallocFailed() ){
    pParse->rc = SQLITE_NOMEM;
  }
  if( pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_DONE && pParse->zErrMsg==0 ){
    sqlite3SetString(&pParse->zErrMsg, sqlite3ErrStr(pParse->rc), (char*)0);
  }
  if( pParse->zErrMsg ){
    if( pzErrMsg && *pzErrMsg==0 ){
      *pzErrMsg = pParse->zErrMsg;
    }else{
      sqliteFree(pParse->zErrMsg);
    }
    pParse->zErrMsg = 0;
    if( !nErr ) nErr++;
  }
  /* if( pParse->pVdbe && pParse->nErr>0 && pParse->nested==0 ){ */
  /*   sqlite3VdbeDelete(pParse->pVdbe); */
  /*   pParse->pVdbe = 0; */
  /* } */
/* #ifndef SQLITE_OMIT_SHARED_CACHE */
/*   if( pParse->nested==0 ){ */
/*     sqliteFree(pParse->aTableLock); */
/*     pParse->aTableLock = 0; */
/*     pParse->nTableLock = 0; */
/*   } */
/* #endif */
  //sqlite3DeleteTable(pParse->db, pParse->pNewTable);
  //sqlite3DeleteTrigger(pParse->pNewTrigger);
 // sqliteFree(pParse->apVarExpr);
  if( nErr>0 && (pParse->rc==SQLITE_OK || pParse->rc==SQLITE_DONE) ){
    pParse->rc = SQLITE_ERROR;
  }
  if( pParse->rc==SQLITE_DONE ) { 
      pParse->rc = SQLITE_OK;
  }
  return nErr;
}

int sqlite3RunParser1(Parse *pParse, const char *zSql, int sqlLen, char **pzErrMsg) {
  int nErr = 0;
  int i;
  void *pEngine;
  int tokenType;
  int lastTokenParsed = -1;
 // sqlite3 *db = pParse->db;
  extern void *sqlite3ParserAlloc(void*(*)(size_t));
  extern void sqlite3ParserFree(void*, void(*)(void*));
  extern void sqlite3Parser(void*, int, Token, Parse*);

  //db->flags &= ~SQLITE_Interrupt;
  pParse->flags &= ~SQLITE_Interrupt;
  pParse->rc = SQLITE_OK;
  i = 0;
  pEngine = sqlite3ParserAlloc((void*(*)(size_t))sqlite3MallocX);
  if( pEngine==0 ){
    return SQLITE_NOMEM;
  }
  assert( pParse->sLastToken.dyn==0 );
  assert( pParse->pNewTable==0 );
  assert( pParse->pNewTrigger==0 );
  assert( pParse->nVar==0 );
  assert( pParse->nVarExpr==0 );
  assert( pParse->nVarExprAlloc==0 );
  assert( pParse->apVarExpr==0 );
  pParse->zTail = pParse->zSql = zSql;
  while( !sqlite3MallocFailed() && /*zSql[i]!=0*/ i < sqlLen ){
    assert( i>=0 );
    pParse->sLastToken.z = (u8*)&zSql[i];
    assert( pParse->sLastToken.dyn==0 );
    pParse->sLastToken.n = getToken((unsigned char*)&zSql[i],&tokenType);
    i += pParse->sLastToken.n;
    
    if (tokenType != TK_SPACE) {
        TokenItem tokenItem;
        tokenItem.token = pParse->sLastToken;
        tokenItem.tokenType = tokenType;
        sqlite3TokenArrayAppend(&pParse->tokens, &tokenItem);
    }

    switch( tokenType ){
      case TK_SPACE:
      case TK_COMMENT: {
        //if( (db->flags & SQLITE_Interrupt)!=0 ){
        if( (pParse->flags & SQLITE_Interrupt)!=0 ){
          pParse->rc = SQLITE_INTERRUPT;
          sqlite3SetString(pzErrMsg, "interrupt", (char*)0);
          goto abort_parse;
        }
        break;
      }
      case TK_ILLEGAL: {
        if( pzErrMsg ){
          sqliteFree(*pzErrMsg);
          *pzErrMsg = sqlite3MPrintf("unrecognized token: \"%T\"",
                          &pParse->sLastToken);
        }
        nErr++;
        goto abort_parse;
      }
      case TK_SEMI: {
        pParse->zTail = &zSql[i];
        /* Fall thru into the default case */
      }
      default: {
        sqlite3Parser(pEngine, tokenType, pParse->sLastToken, pParse);
        lastTokenParsed = tokenType;
        if( pParse->rc!=SQLITE_OK ){
          goto abort_parse;
        }
        break;
      }
    }
  }
abort_parse:
    while(!sqlite3MallocFailed() && i < sqlLen) {
        pParse->sLastToken.z = (u8*)&zSql[i];
        assert( pParse->sLastToken.dyn==0 );
        pParse->sLastToken.n = getToken((unsigned char*)&zSql[i],&tokenType);
        i += pParse->sLastToken.n;

        if (tokenType != TK_SPACE) {
            TokenItem tokenItem;
            tokenItem.token = pParse->sLastToken;
            tokenItem.tokenType = tokenType;
            sqlite3TokenArrayAppend(&pParse->tokens, &tokenItem);
        }
    }  

  if( zSql[i]==0 && nErr==0 && pParse->rc==SQLITE_OK ){
    if( lastTokenParsed!=TK_SEMI ){
      sqlite3Parser(pEngine, TK_SEMI, pParse->sLastToken, pParse);
      pParse->zTail = &zSql[i];
    }
    sqlite3Parser(pEngine, 0, pParse->sLastToken, pParse);
  }
  sqlite3ParserFree(pEngine, sqlite3FreeX);
  if( sqlite3MallocFailed() ){
    pParse->rc = SQLITE_NOMEM;
  }
  if( pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_DONE && pParse->zErrMsg==0 ){
    sqlite3SetString(&pParse->zErrMsg, sqlite3ErrStr(pParse->rc), (char*)0);
  }
  if( pParse->zErrMsg ){
    if( pzErrMsg && *pzErrMsg==0 ){
      *pzErrMsg = pParse->zErrMsg;
    }else{
      sqliteFree(pParse->zErrMsg);
    }
    pParse->zErrMsg = 0;
    if( !nErr ) nErr++;
  }
  /* if( pParse->pVdbe && pParse->nErr>0 && pParse->nested==0 ){ */
  /*   sqlite3VdbeDelete(pParse->pVdbe); */
  /*   pParse->pVdbe = 0; */
  /* } */
/* #ifndef SQLITE_OMIT_SHARED_CACHE */
/*   if( pParse->nested==0 ){ */
/*     sqliteFree(pParse->aTableLock); */
/*     pParse->aTableLock = 0; */
/*     pParse->nTableLock = 0; */
/*   } */
/* #endif */
  //sqlite3DeleteTable(pParse->db, pParse->pNewTable);
  //sqlite3DeleteTrigger(pParse->pNewTrigger);
 // sqliteFree(pParse->apVarExpr);
  if( nErr>0 && (pParse->rc==SQLITE_OK || pParse->rc==SQLITE_DONE) ){
    pParse->rc = SQLITE_ERROR;
  }
  if( pParse->rc==SQLITE_DONE ) { 
      pParse->rc = SQLITE_OK;
  }
  return nErr;
}
/*******************************file from printf.c**************************************/
/*
** Conversion types fall into various categories as defined by the
** following enumeration.
*/
#define etRADIX       1 /* Integer types.  %d, %x, %o, and so forth */
#define etFLOAT       2 /* Floating point.  %f */
#define etEXP         3 /* Exponentional notation. %e and %E */
#define etGENERIC     4 /* Floating or exponential, depending on exponent. %g */
#define etSIZE        5 /* Return number of characters processed so far. %n */
#define etSTRING      6 /* Strings. %s */
#define etDYNSTRING   7 /* Dynamically allocated strings. %z */
#define etPERCENT     8 /* Percent symbol. %% */
#define etCHARX       9 /* Characters. %c */
/* The rest are extensions, not normally found in printf() */
#define etCHARLIT    10 /* Literal characters.  %' */
#define etSQLESCAPE  11 /* Strings with '\'' doubled.  %q */
#define etSQLESCAPE2 12 /* Strings with '\'' doubled and enclosed in '',
                          NULL pointers replaced by SQL NULL.  %Q */
#define etTOKEN      13 /* a pointer to a Token structure */
#define etSRCLIST    14 /* a pointer to a SrcList */
#define etPOINTER    15 /* The %p conversion */


/*
** An "etByte" is an 8-bit unsigned value.
*/
typedef unsigned char etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
typedef struct et_info {   /* Information about each format field */
  char fmttype;            /* The format field code letter */
  etByte base;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte prefix;           /* Offset into aPrefix[] of the prefix string */
} et_info;

/*
** Allowed values for et_info.flags
*/
#define FLAG_SIGNED  1     /* True if the value to convert is signed */
#define FLAG_INTERN  2     /* True if for internal use only */
#define FLAG_STRING  4     /* Allow infinity precision */


/*
** The following table is searched linearly, so it is good to put the
** most frequently used conversion types first.
*/
static const char aDigits[] = "0123456789ABCDEF0123456789abcdef";
static const char aPrefix[] = "-x0\000X0";
static const et_info fmtinfo[] = {
  {  'd', 10, 1, etRADIX,      0,  0 },
  {  's',  0, 4, etSTRING,     0,  0 },
  {  'g',  0, 1, etGENERIC,    30, 0 },
  {  'z',  0, 6, etDYNSTRING,  0,  0 },
  {  'q',  0, 4, etSQLESCAPE,  0,  0 },
  {  'Q',  0, 4, etSQLESCAPE2, 0,  0 },
  {  'c',  0, 0, etCHARX,      0,  0 },
  {  'o',  8, 0, etRADIX,      0,  2 },
  {  'u', 10, 0, etRADIX,      0,  0 },
  {  'x', 16, 0, etRADIX,      16, 1 },
  {  'X', 16, 0, etRADIX,      0,  4 },
#ifndef SQLITE_OMIT_FLOATING_POINT
  {  'f',  0, 1, etFLOAT,      0,  0 },
  {  'e',  0, 1, etEXP,        30, 0 },
  {  'E',  0, 1, etEXP,        14, 0 },
  {  'G',  0, 1, etGENERIC,    14, 0 },
#endif
  {  'i', 10, 1, etRADIX,      0,  0 },
  {  'n',  0, 0, etSIZE,       0,  0 },
  {  '%',  0, 0, etPERCENT,    0,  0 },
  {  'p', 16, 0, etPOINTER,    0,  1 },
  {  'T',  0, 2, etTOKEN,      0,  0 },
  {  'S',  0, 2, etSRCLIST,    0,  0 },
};
#define etNINFO  (sizeof(fmtinfo)/sizeof(fmtinfo[0]))

/*
** If SQLITE_OMIT_FLOATING_POINT is defined, then none of the floating point
** conversions will work.
*/
#ifndef SQLITE_OMIT_FLOATING_POINT
/*
** "*val" is a double such that 0.1 <= *val < 10.0
** Return the ascii code for the leading digit of *val, then
** multiply "*val" by 10.0 to renormalize.
**
** Example:
**     input:     *val = 3.14159
**     output:    *val = 1.4159    function return = '3'
**
** The counter *cnt is incremented each time.  After counter exceeds
** 16 (the number of significant digits in a 64-bit float) '0' is
** always returned.
*/
static int et_getdigit(LONGDOUBLE_TYPE *val, int *cnt){
  int digit;
  LONGDOUBLE_TYPE d;
  if( (*cnt)++ >= 16 ) return '0';
  digit = (int)*val;
  d = digit;
  digit += '0';
  *val = (*val - d)*10.0;
  return digit;
}
#endif /* SQLITE_OMIT_FLOATING_POINT */

/*
** On machines with a small stack size, you can redefine the
** SQLITE_PRINT_BUF_SIZE to be less than 350.  But beware - for
** smaller values some %f conversions may go into an infinite loop.
*/
#ifndef SQLITE_PRINT_BUF_SIZE
# define SQLITE_PRINT_BUF_SIZE 350
#endif
#define etBUFSIZE SQLITE_PRINT_BUF_SIZE  /* Size of the output buffer */

/*
** The root program.  All variations call this core.
**
** INPUTS:
**   func   This is a pointer to a function taking three arguments
**            1. A pointer to anything.  Same as the "arg" parameter.
**            2. A pointer to the list of characters to be output
**               (Note, this list is NOT null terminated.)
**            3. An integer number of characters to be output.
**               (Note: This number might be zero.)
**
**   arg    This is the pointer to anything which will be passed as the
**          first argument to "func".  Use it for whatever you like.
**
**   fmt    This is the format string, as in the usual print.
**
**   ap     This is a pointer to a list of arguments.  Same as in
**          vfprint.
**
** OUTPUTS:
**          The return value is the total number of characters sent to
**          the function "func".  Returns -1 on a error.
**
** Note that the order in which automatic variables are declared below
** seems to make a big difference in determining how fast this beast
** will run.
*/
static int vxprintf(
  void (*func)(void*,const char*,int),     /* Consumer of text */
  void *arg,                         /* First argument to the consumer */
  int useExtended,                   /* Allow extended %-conversions */
  const char *fmt,                   /* Format string */
  va_list ap                         /* arguments */
){
  int c;                     /* Next character in the format string */
  char *bufpt;               /* Pointer to the conversion buffer */
  int precision;             /* Precision of the current field */
  int length;                /* Length of the field */
  int idx;                   /* A general purpose loop counter */
  int count;                 /* Total number of characters output */
  int width;                 /* Width of the current field */
  etByte flag_leftjustify;   /* True if "-" flag is present */
  etByte flag_plussign;      /* True if "+" flag is present */
  etByte flag_blanksign;     /* True if " " flag is present */
  etByte flag_alternateform; /* True if "#" flag is present */
  etByte flag_altform2;      /* True if "!" flag is present */
  etByte flag_zeropad;       /* True if field width constant starts with zero */
  etByte flag_long;          /* True if "l" flag is present */
  etByte flag_longlong;      /* True if the "ll" flag is present */
  etByte done;               /* Loop termination flag */
  sqlite_uint64 longvalue;   /* Value for integer types */
  LONGDOUBLE_TYPE realvalue; /* Value for real types */
  const et_info *infop;      /* Pointer to the appropriate info structure */
  char buf[etBUFSIZE];       /* Conversion buffer */
  char prefix;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  etByte errorflag = 0;      /* True if an error is encountered */
  etByte xtype;              /* Conversion paradigm */
  char *zExtra;              /* Extra memory used for etTCLESCAPE conversions */
  static const char spaces[] =
   "                                                                         ";
#define etSPACESIZE (sizeof(spaces)-1)
#ifndef SQLITE_OMIT_FLOATING_POINT
  int  exp, e2;              /* exponent of real numbers */
  double rounder;            /* Used for rounding floating point values */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */
  etByte flag_exp;           /* True to force display of the exponent */
  int nsd;                   /* Number of significant digits returned */
#endif

  func(arg,"",0);
  count = length = 0;
  bufpt = 0;
  for(; (c=(*fmt))!=0; ++fmt){
    if( c!='%' ){
      int amt;
      bufpt = (char *)fmt;
      amt = 1;
      while( (c=(*++fmt))!='%' && c!=0 ) amt++;
      (*func)(arg,bufpt,amt);
      count += amt;
      if( c==0 ) break;
    }
    if( (c=(*++fmt))==0 ){
      errorflag = 1;
      (*func)(arg,"%",1);
      count++;
      break;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_plussign = flag_blanksign = 
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    do{
      switch( c ){
        case '-':   flag_leftjustify = 1;     break;
        case '+':   flag_plussign = 1;        break;
        case ' ':   flag_blanksign = 1;       break;
        case '#':   flag_alternateform = 1;   break;
        case '!':   flag_altform2 = 1;        break;
        case '0':   flag_zeropad = 1;         break;
        default:    done = 1;                 break;
      }
    }while( !done && (c=(*++fmt))!=0 );
    /* Get the field width */
    width = 0;
    if( c=='*' ){
      width = va_arg(ap,int);
      if( width<0 ){
        flag_leftjustify = 1;
        width = -width;
      }
      c = *++fmt;
    }else{
      while( c>='0' && c<='9' ){
        width = width*10 + c - '0';
        c = *++fmt;
      }
    }
    if( width > etBUFSIZE-10 ){
      width = etBUFSIZE-10;
    }
    /* Get the precision */
    if( c=='.' ){
      precision = 0;
      c = *++fmt;
      if( c=='*' ){
        precision = va_arg(ap,int);
        if( precision<0 ) precision = -precision;
        c = *++fmt;
      }else{
        while( c>='0' && c<='9' ){
          precision = precision*10 + c - '0';
          c = *++fmt;
        }
      }
    }else{
      precision = -1;
    }
    /* Get the conversion type modifier */
    if( c=='l' ){
      flag_long = 1;
      c = *++fmt;
      if( c=='l' ){
        flag_longlong = 1;
        c = *++fmt;
      }else{
        flag_longlong = 0;
      }
    }else{
      flag_long = flag_longlong = 0;
    }
    /* Fetch the info entry for the field */
    infop = 0;
    for(idx=0; idx<etNINFO; idx++){
      if( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        if( useExtended || (infop->flags & FLAG_INTERN)==0 ){
          xtype = infop->type;
        }
        break;
      }
    }
    zExtra = 0;
    if( infop==0 ){
      return -1;
    }


    /* Limit the precision to prevent overflowing buf[] during conversion */
    if( precision>etBUFSIZE-40 && (infop->flags & FLAG_STRING)==0 ){
      precision = etBUFSIZE-40;
    }

    /*
    ** At this point, variables are initialized as follows:
    **
    **   flag_alternateform          TRUE if a '#' is present.
    **   flag_altform2               TRUE if a '!' is present.
    **   flag_plussign               TRUE if a '+' is present.
    **   flag_leftjustify            TRUE if a '-' is present or if the
    **                               field width was negative.
    **   flag_zeropad                TRUE if the width began with 0.
    **   flag_long                   TRUE if the letter 'l' (ell) prefixed
    **                               the conversion character.
    **   flag_longlong               TRUE if the letter 'll' (ell ell) prefixed
    **                               the conversion character.
    **   flag_blanksign              TRUE if a ' ' is present.
    **   width                       The specified field width.  This is
    **                               always non-negative.  Zero is the default.
    **   precision                   The specified precision.  The default
    **                               is -1.
    **   xtype                       The class of the conversion.
    **   infop                       Pointer to the appropriate info struct.
    */
    switch( xtype ){
      case etPOINTER:
        flag_longlong = sizeof(char*)==sizeof(i64);
        flag_long = sizeof(char*)==sizeof(long int);
        /* Fall through into the next case */
      case etRADIX:
        if( infop->flags & FLAG_SIGNED ){
          i64 v;
          if( flag_longlong )   v = va_arg(ap,i64);
          else if( flag_long )  v = va_arg(ap,long int);
          else                  v = va_arg(ap,int);
          if( v<0 ){
            longvalue = -v;
            prefix = '-';
          }else{
            longvalue = v;
            if( flag_plussign )        prefix = '+';
            else if( flag_blanksign )  prefix = ' ';
            else                       prefix = 0;
          }
        }else{
          if( flag_longlong )   longvalue = va_arg(ap,u64);
          else if( flag_long )  longvalue = va_arg(ap,unsigned long int);
          else                  longvalue = va_arg(ap,unsigned int);
          prefix = 0;
        }
        if( longvalue==0 ) flag_alternateform = 0;
        if( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        bufpt = &buf[etBUFSIZE-1];
        {
          register const char *cset;      /* Use registers for speed */
          register int base;
          cset = &aDigits[infop->charset];
          base = infop->base;
          do{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }while( longvalue>0 );
        }
        length = &buf[etBUFSIZE-1]-bufpt;
        for(idx=precision-length; idx>0; idx--){
          *(--bufpt) = '0';                             /* Zero pad */
        }
        if( prefix ) *(--bufpt) = prefix;               /* Add sign */
        if( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          const char *pre;
          char x;
          pre = &aPrefix[infop->prefix];
          if( *bufpt!=pre[0] ){
            for(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
          }
        }
        length = &buf[etBUFSIZE-1]-bufpt;
        break;
      case etFLOAT:
      case etEXP:
      case etGENERIC:
        realvalue = va_arg(ap,double);
#ifndef SQLITE_OMIT_FLOATING_POINT
        if( precision<0 ) precision = 6;         /* Set default precision */
        if( precision>etBUFSIZE/2-10 ) precision = etBUFSIZE/2-10;
        if( realvalue<0.0 ){
          realvalue = -realvalue;
          prefix = '-';
        }else{
          if( flag_plussign )          prefix = '+';
          else if( flag_blanksign )    prefix = ' ';
          else                         prefix = 0;
        }
        if( xtype==etGENERIC && precision>0 ) precision--;
#if 0
        /* Rounding works like BSD when the constant 0.4999 is used.  Wierd! */
        for(idx=precision, rounder=0.4999; idx>0; idx--, rounder*=0.1);
#else
        /* It makes more sense to use 0.5 */
        for(idx=precision, rounder=0.5; idx>0; idx--, rounder*=0.1){}
#endif
        if( xtype==etFLOAT ) realvalue += rounder;
        /* Normalize realvalue to within 10.0 > realvalue >= 1.0 */
        exp = 0;
        if( realvalue>0.0 ){
          while( realvalue>=1e32 && exp<=350 ){ realvalue *= 1e-32; exp+=32; }
          while( realvalue>=1e8 && exp<=350 ){ realvalue *= 1e-8; exp+=8; }
          while( realvalue>=10.0 && exp<=350 ){ realvalue *= 0.1; exp++; }
          while( realvalue<1e-8 && exp>=-350 ){ realvalue *= 1e8; exp-=8; }
          while( realvalue<1.0 && exp>=-350 ){ realvalue *= 10.0; exp--; }
          if( exp>350 || exp<-350 ){
            bufpt = "NaN";
            length = 3;
            break;
          }
        }
        bufpt = buf;
        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        flag_exp = xtype==etEXP;
        if( xtype!=etFLOAT ){
          realvalue += rounder;
          if( realvalue>=10.0 ){ realvalue *= 0.1; exp++; }
        }
        if( xtype==etGENERIC ){
          flag_rtz = !flag_alternateform;
          if( exp<-4 || exp>precision ){
            xtype = etEXP;
          }else{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }else{
          flag_rtz = 0;
        }
        if( xtype==etEXP ){
          e2 = 0;
        }else{
          e2 = exp;
        }
        nsd = 0;
        flag_dp = (precision>0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        if( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        if( e2<0 ){
          *(bufpt++) = '0';
        }else{
          for(; e2>=0; e2--){
            *(bufpt++) = et_getdigit(&realvalue,&nsd);
          }
        }
        /* The decimal point */
        if( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        for(e2++; e2<0 && precision>0; precision--, e2++){
          *(bufpt++) = '0';
        }
        /* Significant digits after the decimal point */
        while( (precision--)>0 ){
          *(bufpt++) = et_getdigit(&realvalue,&nsd);
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        if( flag_rtz && flag_dp ){
          while( bufpt[-1]=='0' ) *(--bufpt) = 0;
          assert( bufpt>buf );
          if( bufpt[-1]=='.' ){
            if( flag_altform2 ){
              *(bufpt++) = '0';
            }else{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        if( flag_exp || (xtype==etEXP && exp) ){
          *(bufpt++) = aDigits[infop->charset];
          if( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }else{
            *(bufpt++) = '+';
          }
          if( exp>=100 ){
            *(bufpt++) = (exp/100)+'0';                /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = exp/10+'0';                     /* 10's digit */
          *(bufpt++) = exp%10+'0';                     /* 1's digit */
        }
        *bufpt = 0;

        /* The converted number is in buf[] and zero terminated. Output it.
        ** Note that the number is in the usual order, not reversed as with
        ** integer conversions. */
        length = bufpt-buf;
        bufpt = buf;

        /* Special case:  Add leading zeros if the flag_zeropad flag is
        ** set and we are not left justified */
        if( flag_zeropad && !flag_leftjustify && length < width){
          int i;
          int nPad = width - length;
          for(i=width; i>=nPad; i--){
            bufpt[i] = bufpt[i-nPad];
          }
          i = prefix!=0;
          while( nPad-- ) bufpt[i++] = '0';
          length = width;
        }
#endif
        break;
      case etSIZE:
        *(va_arg(ap,int*)) = count;
        length = width = 0;
        break;
      case etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        break;
      case etCHARLIT:
      case etCHARX:
        c = buf[0] = (xtype==etCHARX ? va_arg(ap,int) : *++fmt);
        if( precision>=0 ){
          for(idx=1; idx<precision; idx++) buf[idx] = c;
          length = precision;
        }else{
          length =1;
        }
        bufpt = buf;
        break;
      case etSTRING:
      case etDYNSTRING:
        bufpt = va_arg(ap,char*);
        if( bufpt==0 ){
          bufpt = "";
        }else if( xtype==etDYNSTRING ){
          zExtra = bufpt;
        }
        length = strlen(bufpt);
        if( precision>=0 && precision<length ) length = precision;
        break;
      case etSQLESCAPE:
      case etSQLESCAPE2: {
        int i, j, n, ch, isnull;
        int needQuote;
        char *escarg = va_arg(ap,char*);
        isnull = escarg==0;
        if( isnull ) escarg = (xtype==etSQLESCAPE2 ? "NULL" : "(NULL)");
        for(i=n=0; (ch=escarg[i])!=0; i++){
          if( ch=='\'' )  n++;
        }
        needQuote = !isnull && xtype==etSQLESCAPE2;
        n += i + 1 + needQuote*2;
        if( n>etBUFSIZE ){
          bufpt = zExtra = sqliteMalloc( n );
          if( bufpt==0 ) return -1;
        }else{
          bufpt = buf;
        }
        j = 0;
        if( needQuote ) bufpt[j++] = '\'';
        for(i=0; (ch=escarg[i])!=0; i++){
          bufpt[j++] = ch;
          if( ch=='\'' ) bufpt[j++] = ch;
        }
        if( needQuote ) bufpt[j++] = '\'';
        bufpt[j] = 0;
        length = j;
        /* The precision is ignored on %q and %Q */
        /* if( precision>=0 && precision<length ) length = precision; */
        break;
      }
      case etTOKEN: {
        Token *pToken = va_arg(ap, Token*);
        if( pToken && pToken->z ){
          (*func)(arg, (char*)pToken->z, pToken->n);
        }
        length = width = 0;
        break;
      }
      case etSRCLIST: {
        SrcList *pSrc = va_arg(ap, SrcList*);
        int k = va_arg(ap, int);
        struct SrcList_item *pItem = &pSrc->a[k];
        assert( k>=0 && k<pSrc->nSrc );
        if( pItem->zDatabase && pItem->zDatabase[0] ){
          (*func)(arg, pItem->zDatabase, strlen(pItem->zDatabase));
          (*func)(arg, ".", 1);
        }
        (*func)(arg, pItem->zName, strlen(pItem->zName));
        length = width = 0;
        break;
      }
    }/* End switch over the format type */
    /*
    ** The text of the conversion is pointed to by "bufpt" and is
    ** "length" characters long.  The field width is "width".  Do
    ** the output.
    */
    if( !flag_leftjustify ){
      register int nspace;
      nspace = width-length;
      if( nspace>0 ){
        count += nspace;
        while( nspace>=etSPACESIZE ){
          (*func)(arg,spaces,etSPACESIZE);
          nspace -= etSPACESIZE;
        }
        if( nspace>0 ) (*func)(arg,spaces,nspace);
      }
    }
    if( length>0 ){
      (*func)(arg,bufpt,length);
      count += length;
    }
    if( flag_leftjustify ){
      register int nspace;
      nspace = width-length;
      if( nspace>0 ){
        count += nspace;
        while( nspace>=etSPACESIZE ){
          (*func)(arg,spaces,etSPACESIZE);
          nspace -= etSPACESIZE;
        }
        if( nspace>0 ) (*func)(arg,spaces,nspace);
      }
    }
    if( zExtra ){
      sqliteFree(zExtra);
    }
  }/* End for loop over the format string */
  return errorflag ? -1 : count;
} /* End of function */


/* This structure is used to store state information about the
** write to memory that is currently in progress.
*/
struct sgMprintf {
  char *zBase;     /* A base allocation */
  char *zText;     /* The string collected so far */
  int  nChar;      /* Length of the string so far */
  int  nTotal;     /* Output size if unconstrained */
  int  nAlloc;     /* Amount of space allocated in zText */
  void *(*xRealloc)(void*,int);  /* Function used to realloc memory */
};

/* 
** This function implements the callback from vxprintf. 
**
** This routine add nNewChar characters of text in zNewText to
** the sgMprintf structure pointed to by "arg".
*/
static void mout(void *arg, const char *zNewText, int nNewChar){
  struct sgMprintf *pM = (struct sgMprintf*)arg;
  pM->nTotal += nNewChar;
  if( pM->nChar + nNewChar + 1 > pM->nAlloc ){
    if( pM->xRealloc==0 ){
      nNewChar =  pM->nAlloc - pM->nChar - 1;
    }else{
      pM->nAlloc = pM->nChar + nNewChar*2 + 1;
      if( pM->zText==pM->zBase ){
        pM->zText = pM->xRealloc(0, pM->nAlloc);
        if( pM->zText && pM->nChar ){
          memcpy(pM->zText, pM->zBase, pM->nChar);
        }
      }else{
        char *zNew;
        zNew = pM->xRealloc(pM->zText, pM->nAlloc);
        if( zNew ){
          pM->zText = zNew;
        }
      }
    }
  }
  if( pM->zText ){
    if( nNewChar>0 ){
      memcpy(&pM->zText[pM->nChar], zNewText, nNewChar);
      pM->nChar += nNewChar;
    }
    pM->zText[pM->nChar] = 0;
  }
}

/*
** This routine is a wrapper around xprintf() that invokes mout() as
** the consumer.  
*/
static char *base_vprintf(
  void *(*xRealloc)(void*,int),   /* Routine to realloc memory. May be NULL */
  int useInternal,                /* Use internal %-conversions if true */
  char *zInitBuf,                 /* Initially write here, before mallocing */
  int nInitBuf,                   /* Size of zInitBuf[] */
  const char *zFormat,            /* format string */
  va_list ap                      /* arguments */
){
  struct sgMprintf sM;
  sM.zBase = sM.zText = zInitBuf;
  sM.nChar = sM.nTotal = 0;
  sM.nAlloc = nInitBuf;
  sM.xRealloc = xRealloc;
  vxprintf(mout, &sM, useInternal, zFormat, ap);
  if( xRealloc ){
    if( sM.zText==sM.zBase ){
      sM.zText = xRealloc(0, sM.nChar+1);
      if( sM.zText ){
        memcpy(sM.zText, sM.zBase, sM.nChar+1);
      }
    }else if( sM.nAlloc>sM.nChar+10 ){
      char *zNew = xRealloc(sM.zText, sM.nChar+1);
      if( zNew ){
        sM.zText = zNew;
      }
    }
  }
  return sM.zText;
}

/*
** Realloc that is a real function, not a macro.
*/
static void *printf_realloc(void *old, int size){
  return sqliteRealloc(old,size);
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
char *sqlite3VMPrintf(const char *zFormat, va_list ap){
  char zBase[SQLITE_PRINT_BUF_SIZE];
  return base_vprintf(printf_realloc, 1, zBase, sizeof(zBase), zFormat, ap);
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
char *sqlite3MPrintf(const char *zFormat, ...){
  va_list ap;
  char *z;
  char zBase[SQLITE_PRINT_BUF_SIZE];
  va_start(ap, zFormat);
  z = base_vprintf(printf_realloc, 1, zBase, sizeof(zBase), zFormat, ap);
  va_end(ap);
  return z;
}

/*
** Print into memory obtained from malloc().  Do not use the internal
** %-conversion extensions.  This routine is for use by external users.
*/
char *sqlite3_mprintf(const char *zFormat, ...){
  va_list ap;
  char *z;
  char zBuf[200];

  va_start(ap,zFormat);
  z = base_vprintf((void*(*)(void*,int))realloc, 0, 
                   zBuf, sizeof(zBuf), zFormat, ap);
  va_end(ap);
  return z;
}

/* This is the varargs version of sqlite3_mprintf.  
*/
char *sqlite3_vmprintf(const char *zFormat, va_list ap){
  char zBuf[200];
  return base_vprintf((void*(*)(void*,int))realloc, 0,
                      zBuf, sizeof(zBuf), zFormat, ap);
}

/*
** sqlite3_snprintf() works like snprintf() except that it ignores the
** current locale settings.  This is important for SQLite because we
** are not able to use a "," as the decimal point in place of "." as
** specified by some locales.
*/
char *sqlite3_snprintf(int n, char *zBuf, const char *zFormat, ...){
  char *z;
  va_list ap;

  va_start(ap,zFormat);
  z = base_vprintf(0, 0, zBuf, n, zFormat, ap);
  va_end(ap);
  return z;
}

#if defined(SQLITE_TEST) || defined(SQLITE_DEBUG)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
void sqlite3DebugPrintf(const char *zFormat, ...){
  extern int getpid(void);
  va_list ap;
  char zBuf[500];
  va_start(ap, zFormat);
  base_vprintf(0, 0, zBuf, sizeof(zBuf), zFormat, ap);
  va_end(ap);
  fprintf(stdout,"%d: %s", getpid(), zBuf);
  fflush(stdout);
}
#endif
/*******************************file from expr.c**************************************/
/*
** Return the 'affinity' of the expression pExpr if any.
**
** If pExpr is a column, a reference to a column via an 'AS' alias,
** or a sub-select with a column as the return value, then the 
** affinity of that column is returned. Otherwise, 0x00 is returned,
** indicating no affinity for the expression.
**
** i.e. the WHERE clause expresssions in the following statements all
** have an affinity:
**
** CREATE TABLE t1(a);
** SELECT * FROM t1 WHERE a;
** SELECT a AS b FROM t1 WHERE b;
** SELECT * FROM t1 WHERE (select a from t1);
*/
char sqlite3ExprAffinity(Expr *pExpr){
  int op = pExpr->op;
  if( op==TK_AS ){
    return sqlite3ExprAffinity(pExpr->pLeft);
  }
  if( op==TK_SELECT ){
    return sqlite3ExprAffinity(pExpr->pSelect->pEList->a[0].pExpr);
  }
#ifndef SQLITE_OMIT_CAST
  if( op==TK_CAST ){
    return sqlite3AffinityType(&pExpr->token);
  }
#endif
  return pExpr->affinity;
}


char sqlite3CompareAffinity(Expr *pExpr, char aff2){
  char aff1 = sqlite3ExprAffinity(pExpr);
  if( aff1 && aff2 ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    if( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      return SQLITE_AFF_NUMERIC;
    }else{
      return SQLITE_AFF_NONE;
    }
  }else if( !aff1 && !aff2 ){
    /* Neither side of the comparison is a column.  Compare the
    ** results directly.
    */
    return SQLITE_AFF_NONE;
  }else{
    /* One side is a column, the other is not. Use the columns affinity. */
    assert( aff1==0 || aff2==0 );
    return (aff1 + aff2);
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
static char comparisonAffinity(Expr *pExpr){
  char aff;
  assert( pExpr->op==TK_EQ || pExpr->op==TK_IN || pExpr->op==TK_LT ||
          pExpr->op==TK_GT || pExpr->op==TK_GE || pExpr->op==TK_LE ||
          pExpr->op==TK_NE );
  assert( pExpr->pLeft );
  aff = sqlite3ExprAffinity(pExpr->pLeft);
  if( pExpr->pRight ){
    aff = sqlite3CompareAffinity(pExpr->pRight, aff);
  }
  else if( pExpr->pSelect ){
    aff = sqlite3CompareAffinity(pExpr->pSelect->pEList->a[0].pExpr, aff);
  }
  else if( !aff ){
    aff = SQLITE_AFF_NUMERIC;
  }
  return aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
int sqlite3IndexAffinityOk(Expr *pExpr, char idx_affinity){
  char aff = comparisonAffinity(pExpr);
  switch( aff ){
    case SQLITE_AFF_NONE:
      return 1;
    case SQLITE_AFF_TEXT:
      return idx_affinity==SQLITE_AFF_TEXT;
    default:
      return sqlite3IsNumericAffinity(idx_affinity);
  }
}

/*
** Return the P1 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
** If jumpIfNull is true, then set the low byte of the returned
** P1 value to tell the opcode to jump if either expression
** evaluates to NULL.
*/
static int binaryCompareP1(Expr *pExpr1, Expr *pExpr2, int jumpIfNull){
  char aff = sqlite3ExprAffinity(pExpr2);
  return ((int)sqlite3CompareAffinity(pExpr1, aff))+(jumpIfNull?0x100:0);
}

Expr *sqlite3Expr(int op, Expr *pLeft, Expr *pRight, const Token *pToken){
  Expr *pNew;
  pNew = sqliteMalloc( sizeof(Expr) );
  if( pNew==0 ){
    /* When malloc fails, delete pLeft and pRight. Expressions passed to 
    ** this function must always be allocated with sqlite3Expr() for this 
    ** reason. 
    */
    sqlite3ExprDelete(pLeft);
    sqlite3ExprDelete(pRight);
    return 0;
  }
  pNew->op = op;
  pNew->pLeft = pLeft;
  pNew->pRight = pRight;
  pNew->pParent = NULL;
 
  if (pLeft) {
      pLeft->pParent = pNew;
  }

  if (pRight) {
      pRight->pParent = pNew;
  }

  pNew->iAgg = -1;
  if( pToken ){
    assert( pToken->dyn==0 );
    pNew->span = pNew->token = *pToken;
  }else if( pLeft && pRight ){
    sqlite3ExprSpan(pNew, &pLeft->span, &pRight->span);
  }
  return pNew;
}

Expr *sqlite3RegisterExpr(Parse *pParse, Token *pToken){
 
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
*/
Expr *sqlite3ExprAnd(Expr *pLeft, Expr *pRight){
  if( pLeft==0 ){
    return pRight;
  }else if( pRight==0 ){
    return pLeft;
  }else{
    return sqlite3Expr(TK_AND, pLeft, pRight, 0);
  }
}

/*
** Set the Expr.span field of the given expression to span all
** text between the two given tokens.
*/
void sqlite3ExprSpan(Expr *pExpr, Token *pLeft, Token *pRight){
  assert( pRight!=0 );
  assert( pLeft!=0 );
  if( !sqlite3MallocFailed() && pRight->z && pLeft->z ){
    assert( pLeft->dyn==0 || pLeft->z[pLeft->n]==0 );
    if( pLeft->dyn==0 && pRight->dyn==0 ){
      pExpr->span.z = pLeft->z;
      pExpr->span.n = pRight->n + (pRight->z - pLeft->z);
    }else{
      pExpr->span.z = 0;
    }
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
Expr *sqlite3ExprFunction(ExprList *pList, Token *pToken){
    Expr *pNew;
    assert( pToken );
    pNew = sqliteMalloc( sizeof(Expr) );
    if( pNew==0 ){
        sqlite3ExprListDelete(pList); /* Avoid leaking memory when malloc fails */
        return 0;
    }
    pNew->op = TK_FUNCTION;
    pNew->pList = pList;
    assert( pToken->dyn==0 );
    pNew->token = *pToken;
    pNew->span = pNew->token;
    return pNew;
}

Expr *sqlite3ExprLikeOp(ExprList *pList, Token *pToken) {
    Expr *pNew;
    assert( pToken );
    pNew = sqliteMalloc( sizeof(Expr) );
    if( pNew==0 ){
        sqlite3ExprListDelete(pList); /* Avoid leaking memory when malloc fails */
        return 0;
    }
    pNew->op = TK_LIKE_KW;
    pNew->pList = pList;
    assert( pToken->dyn==0 );
    pNew->token = *pToken;
    pNew->span = pNew->token;
    return pNew;
   
}
void sqlite3ExprAssignVarNumber(Parse *pParse, Expr *pExpr){

}

void sqlite3ExprDelete(Expr *p){
  if( p==0 ) return;
  if( p->span.dyn ) sqliteFree((char*)p->span.z);
  if( p->token.dyn ) sqliteFree((char*)p->token.z);
  sqlite3ExprDelete(p->pLeft);
  sqlite3ExprDelete(p->pRight);
  sqlite3ExprListDelete(p->pList);
  sqlite3SelectDelete(p->pSelect);
  sqliteFree(p);
}

/*
** The Expr.token field might be a string literal that is quoted.
** If so, remove the quotation marks.
*/
void sqlite3DequoteExpr(Expr *p){
  if( ExprHasAnyProperty(p, EP_Dequoted) ){
    return;
  }
  ExprSetProperty(p, EP_Dequoted);
  if( p->token.dyn==0 ){
    sqlite3TokenCopy(&p->token, &p->token);
  }
  sqlite3Dequote((char*)p->token.z);
}

/*
** The following group of routines make deep copies of expressions,
** expression lists, ID lists, and select statements.  The copies can
** be deleted (by being passed to their respective ...Delete() routines)
** without effecting the originals.
**
** The expression list, ID, and source lists return by sqlite3ExprListDup(),
** sqlite3IdListDup(), and sqlite3SrcListDup() can not be further expanded 
** by subsequent calls to sqlite*ListAppend() routines.
**
** Any tables that the SrcList might point to are not duplicated.
*/
Expr *sqlite3ExprDup(Expr *p){
  Expr *pNew;
  if( p==0 ) return 0;
  pNew = sqliteMallocRaw( sizeof(*p) );
  if( pNew==0 ) return 0;
  memcpy(pNew, p, sizeof(*pNew));
  if( p->token.z!=0 ){
    pNew->token.z = (u8*)sqliteStrNDup((char*)p->token.z, p->token.n);
    pNew->token.dyn = 1;
  }else{
    assert( pNew->token.z==0 );
  }
  pNew->span.z = 0;
  pNew->pLeft = sqlite3ExprDup(p->pLeft);
  pNew->pRight = sqlite3ExprDup(p->pRight);
  pNew->pList = sqlite3ExprListDup(p->pList);
  pNew->pSelect = sqlite3SelectDup(p->pSelect);
  pNew->pTab = p->pTab;
  return pNew;
}
void sqlite3TokenCopy(Token *pTo, Token *pFrom){
  if( pTo->dyn ) sqliteFree((char*)pTo->z);
  if( pFrom->z ){
    pTo->n = pFrom->n;
    pTo->z = (u8*)sqliteStrNDup((char*)pFrom->z, pFrom->n);
    pTo->dyn = 1;
  }else{
    pTo->z = 0;
  }
}
ExprList *sqlite3ExprListDup(ExprList *p){
  ExprList *pNew;
  struct ExprList_item *pItem, *pOldItem;
  int i;
  if( p==0 ) return 0;
  pNew = sqliteMalloc( sizeof(*pNew) );
  if( pNew==0 ) return 0;
  pNew->nExpr = pNew->nAlloc = p->nExpr;
  pNew->a = pItem = sqliteMalloc( p->nExpr*sizeof(p->a[0]) );
  if( pItem==0 ){
    sqliteFree(pNew);
    return 0;
  } 
  pOldItem = p->a;
  for(i=0; i<p->nExpr; i++, pItem++, pOldItem++){
    Expr *pNewExpr, *pOldExpr;
    pItem->pExpr = pNewExpr = sqlite3ExprDup(pOldExpr = pOldItem->pExpr);
    if( pOldExpr->span.z!=0 && pNewExpr ){
      /* Always make a copy of the span for top-level expressions in the
      ** expression list.  The logic in SELECT processing that determines
      ** the names of columns in the result set needs this information */
      sqlite3TokenCopy(&pNewExpr->span, &pOldExpr->span);
    }
    assert( pNewExpr==0 || pNewExpr->span.z!=0 
            || pOldExpr->span.z==0
            || sqlite3MallocFailed() );
    pItem->zName = sqliteStrDup(pOldItem->zName);
    pItem->sortOrder = pOldItem->sortOrder;
    pItem->isAgg = pOldItem->isAgg;
    pItem->done = 0;
  }
  return pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for 
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#if !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SrcList *sqlite3SrcListDup(SrcList *p){
  SrcList *pNew;
  int i;
  int nByte;
  if( p==0 ) return 0;
  nByte = sizeof(*p) + (p->nSrc>0 ? sizeof(p->a[0]) * (p->nSrc-1) : 0);
  pNew = sqliteMallocRaw( nByte );
  if( pNew==0 ) return 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  for(i=0; i<p->nSrc; i++){
    struct SrcList_item *pNewItem = &pNew->a[i];
    struct SrcList_item *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->zDatabase = sqliteStrDup(pOldItem->zDatabase);
    pNewItem->zName = sqliteStrDup(pOldItem->zName);
    pNewItem->zAlias = sqliteStrDup(pOldItem->zAlias);
    pNewItem->jointype = pOldItem->jointype;
    pNewItem->iCursor = pOldItem->iCursor;
    pNewItem->isPopulated = pOldItem->isPopulated;
    pTab = pNewItem->pTab = pOldItem->pTab;
    if( pTab ){
      pTab->nRef++;
    }
    pNewItem->pSelect = sqlite3SelectDup(pOldItem->pSelect);
    pNewItem->pOn = sqlite3ExprDup(pOldItem->pOn);
    pNewItem->pUsing = sqlite3IdListDup(pOldItem->pUsing);
    pNewItem->colUsed = pOldItem->colUsed;
  }
  return pNew;
}
IdList *sqlite3IdListDup(IdList *p){
  IdList *pNew;
  int i;
  if( p==0 ) return 0;
  pNew = sqliteMallocRaw( sizeof(*pNew) );
  if( pNew==0 ) return 0;
  pNew->nId = pNew->nAlloc = p->nId;
  pNew->a = sqliteMallocRaw( p->nId*sizeof(p->a[0]) );
  if( pNew->a==0 ){
    sqliteFree(pNew);
    return 0;
  }
  for(i=0; i<p->nId; i++){
    struct IdList_item *pNewItem = &pNew->a[i];
    struct IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqliteStrDup(pOldItem->zName);
    pNewItem->idx = pOldItem->idx;
  }
  return pNew;
}
Select *sqlite3SelectDup(Select *p){
  Select *pNew;
  if( p==0 ) return 0;
  pNew = sqliteMallocRaw( sizeof(*p) );
  if( pNew==0 ) return 0;
  pNew->isDistinct = p->isDistinct;
  pNew->pEList = sqlite3ExprListDup(p->pEList);
  pNew->pSrc = sqlite3SrcListDup(p->pSrc);
  pNew->pWhere = sqlite3ExprDup(p->pWhere);
  pNew->pGroupBy = sqlite3ExprListDup(p->pGroupBy);
  pNew->pHaving = sqlite3ExprDup(p->pHaving);
  pNew->pOrderBy = sqlite3ExprListDup(p->pOrderBy);
  pNew->op = p->op;
  pNew->pPrior = sqlite3SelectDup(p->pPrior);
  pNew->pLimit = sqlite3ExprDup(p->pLimit);
  pNew->pOffset = sqlite3ExprDup(p->pOffset);
  pNew->iLimit = -1;
  pNew->iOffset = -1;
  pNew->isResolved = p->isResolved;
  pNew->isAgg = p->isAgg;
  pNew->usesVirt = 0;
  pNew->disallowOrderBy = 0;
  pNew->pRightmost = 0;
  pNew->addrOpenVirt[0] = -1;
  pNew->addrOpenVirt[1] = -1;
  pNew->addrOpenVirt[2] = -1;
  return pNew;
}
#else
Select *sqlite3SelectDup(Select *p){
  assert( p==0 );
  return 0;
}
#endif


/*
** Add a new element to the end of an expression list.  If pList is
** initially NULL, then create a new expression list.
*/
ExprList *sqlite3ExprListAppend(ExprList *pList, Expr *pExpr, Token *pName){
  if( pList==0 ){
    pList = sqliteMalloc( sizeof(ExprList) );
    if( pList==0 ){
      goto no_mem;
    }
    assert( pList->nAlloc==0 );
  }
  if( pList->nAlloc<=pList->nExpr ){
    struct ExprList_item *a;
    int n = pList->nAlloc*2 + 4;
    a = sqliteRealloc(pList->a, n*sizeof(pList->a[0]));
    if( a==0 ){
      goto no_mem;
    }
    pList->a = a;
    pList->nAlloc = n;
  }
  assert( pList->a!=0 );
  if( pExpr || pName ){
    struct ExprList_item *pItem = &pList->a[pList->nExpr++];
    memset(pItem, 0, sizeof(*pItem));
    pItem->zName = sqlite3NameFromToken(pName);
    pItem->pExpr = pExpr;
  }
  return pList;

no_mem:     
  /* Avoid leaking memory if malloc has failed. */
  sqlite3ExprDelete(pExpr);
  sqlite3ExprListDelete(pList);
  return 0;
}

/*
** Delete an entire expression list.
*/
void sqlite3ExprListDelete(ExprList *pList){
  int i;
  struct ExprList_item *pItem;
  if( pList==0 ) return;
  assert( pList->a!=0 || (pList->nExpr==0 && pList->nAlloc==0) );
  assert( pList->nExpr<=pList->nAlloc );
  for(pItem=pList->a, i=0; i<pList->nExpr; i++, pItem++){
    sqlite3ExprDelete(pItem->pExpr);
    sqliteFree(pItem->zName);
  }
  sqliteFree(pList->a);
  sqliteFree(pList);
}

ValuesList *sqlite3ValuesListAppend(ValuesList *valueList, ExprList* exprList) {
    if (exprList == NULL) { return valueList; }   
    if (valueList == NULL) {
        valueList = sqliteMalloc(sizeof(ValuesList));
        if (valueList == NULL) { goto no_mem; }
    }
    
    if (valueList->nAlloc <= valueList->nValues) {
        int allocNum = valueList->nAlloc * 2 + 4;
        ExprList** a = valueList->a;
        a = sqliteRealloc(valueList->a, allocNum * sizeof(ExprList*));
        if (a == NULL) {
            goto no_mem;
        }
        valueList->a = a;
        valueList->nAlloc = allocNum;
    }

    valueList->a[valueList->nValues++] = exprList;
    return valueList;

no_mem:
    sqlite3ExprListDelete(exprList);
    sqlite3ValuesListDelete(valueList);
    return NULL;
}

void sqlite3ValuesListDelete(ValuesList* valuesList) {
    if (valuesList == NULL) { return; }
    int i = 0;
    for (i = 0; i < valuesList->nValues; i++) {
        sqlite3ExprListDelete(valuesList->a[i]);
    }
    sqliteFree(valuesList->a);
    sqliteFree(valuesList);
}

/*
** Walk an expression tree.  Call xFunc for each node visited.
**
** The return value from xFunc determines whether the tree walk continues.
** 0 means continue walking the tree.  1 means do not walk children
** of the current node but continue with siblings.  2 means abandon
** the tree walk completely.
**
** The return value from this routine is 1 to abandon the tree walk
** and 0 to continue.
**
** NOTICE:  This routine does *not* descend into subqueries.
*/
static int walkExprList(ExprList *, int (*)(void *, Expr*), void *);
static int walkExprTree(Expr *pExpr, int (*xFunc)(void*,Expr*), void *pArg){
  int rc;
  if( pExpr==0 ) return 0;
  rc = (*xFunc)(pArg, pExpr);
  if( rc==0 ){
    if( walkExprTree(pExpr->pLeft, xFunc, pArg) ) return 1;
    if( walkExprTree(pExpr->pRight, xFunc, pArg) ) return 1;
    if( walkExprList(pExpr->pList, xFunc, pArg) ) return 1;
  }
  return rc>1;
}

/*
** Call walkExprTree() for every expression in list p.
*/
static int walkExprList(ExprList *p, int (*xFunc)(void *, Expr*), void *pArg){
  int i;
  struct ExprList_item *pItem;
  if( !p ) return 0;
  for(i=p->nExpr, pItem=p->a; i>0; i--, pItem++){
    if( walkExprTree(pItem->pExpr, xFunc, pArg) ) return 1;
  }
  return 0;
}

/*
** Call walkExprTree() for every expression in Select p, not including
** expressions that are part of sub-selects in any FROM clause or the LIMIT
** or OFFSET expressions..
*/
static int walkSelectExpr(Select *p, int (*xFunc)(void *, Expr*), void *pArg){
  walkExprList(p->pEList, xFunc, pArg);
  walkExprTree(p->pWhere, xFunc, pArg);
  walkExprList(p->pGroupBy, xFunc, pArg);
  walkExprTree(p->pHaving, xFunc, pArg);
  walkExprList(p->pOrderBy, xFunc, pArg);
  return 0;
}


/*
** This routine is designed as an xFunc for walkExprTree().
**
** pArg is really a pointer to an integer.  If we can tell by looking
** at pExpr that the expression that contains pExpr is not a constant
** expression, then set *pArg to 0 and return 2 to abandon the tree walk.
** If pExpr does does not disqualify the expression from being a constant
** then do nothing.
**
** After walking the whole tree, if no nodes are found that disqualify
** the expression as constant, then we assume the whole expression
** is constant.  See sqlite3ExprIsConstant() for additional information.
*/
static int exprNodeIsConstant(void *pArg, Expr *pExpr){
  switch( pExpr->op ){
    /* Consider functions to be constant if all their arguments are constant
    ** and *pArg==2 */
    case TK_FUNCTION:
      if( *((int*)pArg)==2 ) return 0;
      /* Fall through */
    case TK_ID:
    case TK_COLUMN:
    case TK_DOT:
    case TK_AGG_FUNCTION:
    case TK_AGG_COLUMN:
#ifndef SQLITE_OMIT_SUBQUERY
    case TK_SELECT:
    case TK_EXISTS:
#endif
      *((int*)pArg) = 0;
      return 2;
    case TK_IN:
      if( pExpr->pSelect ){
        *((int*)pArg) = 0;
        return 2;
      }
    default:
      return 0;
  }
}

/*
** Walk an expression tree.  Return 1 if the expression is constant
** and 0 if it involves variables or function calls.
**
** For the purposes of this function, a double-quoted string (ex: "abc")
** is considered a variable but a single-quoted string (ex: 'abc') is
** a constant.
*/
int sqlite3ExprIsConstant(Expr *p){
  int isConst = 1;
  walkExprTree(p, exprNodeIsConstant, &isConst);
  return isConst;
}

/*
** Walk an expression tree.  Return 1 if the expression is constant
** or a function call with constant arguments.  Return and 0 if there
** are any variables.
**
** For the purposes of this function, a double-quoted string (ex: "abc")
** is considered a variable but a single-quoted string (ex: 'abc') is
** a constant.
*/
int sqlite3ExprIsConstantOrFunction(Expr *p){
  int isConst = 2;
  walkExprTree(p, exprNodeIsConstant, &isConst);
  return isConst!=0;
}

/*
** If the expression p codes a constant integer that is small enough
** to fit in a 32-bit integer, return 1 and put the value of the integer
** in *pValue.  If the expression is not an integer or if it is too big
** to fit in a signed 32-bit integer, return 0 and leave *pValue unchanged.
*/
int sqlite3ExprIsInteger(Expr *p, int *pValue){
  switch( p->op ){
    case TK_INTEGER: {
      if( sqlite3GetInt32((char*)p->token.z, pValue) ){
        return 1;
      }
      break;
    }
    case TK_UPLUS: {
      return sqlite3ExprIsInteger(p->pLeft, pValue);
    }
    case TK_UMINUS: {
      int v;
      if( sqlite3ExprIsInteger(p->pLeft, &v) ){
        *pValue = -v;
        return 1;
      }
      break;
    }
    default: break;
  }
  return 0;
}

/*
** Return TRUE if the given string is a row-id column name.
*/
int sqlite3IsRowid(const char *z){
  if( sqlite3StrICmp(z, "_ROWID_")==0 ) return 1;
  if( sqlite3StrICmp(z, "ROWID")==0 ) return 1;
  if( sqlite3StrICmp(z, "OID")==0 ) return 1;
  return 0;
}

/*
** A pointer instance of this structure is used to pass information
** through walkExprTree into codeSubqueryStep().
*/
typedef struct QueryCoder QueryCoder;
struct QueryCoder {
  Parse *pParse;       /* The parsing context */
  NameContext *pNC;    /* Namespace of first enclosing query */
};

#ifndef SQLITE_OMIT_TRIGGER

#endif

/*
** Do a deep comparison of two expression trees.  Return TRUE (non-zero)
** if they are identical and return FALSE if they differ in any way.
*/
int sqlite3ExprCompare(Expr *pA, Expr *pB){
  int i;
  if( pA==0||pB==0 ){
    return pB==pA;
  }
  if( pA->op!=pB->op ) return 0;
  if( (pA->flags & EP_Distinct)!=(pB->flags & EP_Distinct) ) return 0;
  if( !sqlite3ExprCompare(pA->pLeft, pB->pLeft) ) return 0;
  if( !sqlite3ExprCompare(pA->pRight, pB->pRight) ) return 0;
  if( pA->pList ){
    if( pB->pList==0 ) return 0;
    if( pA->pList->nExpr!=pB->pList->nExpr ) return 0;
    for(i=0; i<pA->pList->nExpr; i++){
      if( !sqlite3ExprCompare(pA->pList->a[i].pExpr, pB->pList->a[i].pExpr) ){
        return 0;
      }
    }
  }else if( pB->pList ){
    return 0;
  }
  if( pA->pSelect || pB->pSelect ) return 0;
  if( pA->iTable!=pB->iTable || pA->iColumn!=pB->iColumn ) return 0;
  if( pA->token.z ){
    if( pB->token.z==0 ) return 0;
    if( pB->token.n!=pA->token.n ) return 0;
    if( sqlite3StrNICmp((char*)pA->token.z,(char*)pB->token.z,pB->token.n)!=0 ){
      return 0;
    }
  }
  return 1;
}


/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
static int addAggInfoColumn(AggInfo *pInfo){
  int i;
  i = sqlite3ArrayAllocate((void**)&pInfo->aCol, sizeof(pInfo->aCol[0]), 3);
  if( i<0 ){
    return -1;
  }
  return i;
}    

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
static int addAggInfoFunc(AggInfo *pInfo){
  int i;
  i = sqlite3ArrayAllocate((void**)&pInfo->aFunc, sizeof(pInfo->aFunc[0]), 2);
  if( i<0 ){
    return -1;
  }
  return i;
}
/*******************************file from build.c**************************************/
/*
** This routine is called when a new SQL statement is beginning to
** be parsed.  Initialize the pParse structure as needed.
*/
void sqlite3BeginParse(Parse *pParse, int explainFlag){
  pParse->explain = explainFlag;
  //pParse->nVar = 0;
}

#ifndef SQLITE_OMIT_SHARED_CACHE
/*
** The TableLock structure is only used by the sqlite3TableLock() and
** codeTableLocks() functions.
*/
struct TableLock {
  int iDb;             /* The database containing the table to be locked */
  int iTab;            /* The root page of the table to be locked */
  u8 isWriteLock;      /* True for write lock.  False for a read lock */
  const char *zName;   /* Name of the table */
};

/*
** Record the fact that we want to lock a table at run-time.  
**
** The table to be locked has root page iTab and is found in database iDb.
** A read or a write lock can be taken depending on isWritelock.
**
** This routine just records the fact that the lock is desired.  The
** code to make the lock occur is generated by a later call to
** codeTableLocks() which occurs during sqlite3FinishCoding().
*/
void sqlite3TableLock(
  Parse *pParse,     /* Parsing context */
  int iDb,           /* Index of the database containing the table to lock */
  int iTab,          /* Root page number of the table to be locked */
  u8 isWriteLock,    /* True for a write lock */
  const char *zName  /* Name of the table to be locked */
){

}

/*
** Code an OP_TableLock instruction for each table locked by the
** statement (configured by calls to sqlite3TableLock()).
*/
/* static void codeTableLocks(Parse *pParse){ */
/*   int i; */
/*   Vdbe *pVdbe; */ 
/*   assert( sqlite3ThreadDataReadOnly()->useSharedData || pParse->nTableLock==0 ); */

/*   if( 0==(pVdbe = sqlite3GetVdbe(pParse)) ){ */
/*     return; */
/*   } */

/*   for(i=0; i<pParse->nTableLock; i++){ */
/*     TableLock *p = &pParse->aTableLock[i]; */
/*     int p1 = p->iDb; */
/*     if( p->isWriteLock ){ */
/*       p1 = -1*(p1+1); */
/*     } */
/*     sqlite3VdbeOp3(pVdbe, OP_TableLock, p1, p->iTab, p->zName, P3_STATIC); */
/*   } */
/* } */
#else
  #define codeTableLocks(x)
#endif

/*
** This routine is called after a single SQL statement has been
** parsed and a VDBE program to execute that statement has been
** prepared.  This routine puts the finishing touches on the
** VDBE program and resets the pParse structure for the next
** parse.
**
** Note that if an error occurred, it might be the case that
** no VDBE code was generated.
*/
void sqlite3FinishCoding(Parse *pParse){

}


/*
** Reclaim the memory used by an index
*/
static void freeIndex(Index *p){
  sqliteFree(p->zColAff);
  sqliteFree(p);
}

/*
** This routine is called when a commit occurs.
*/
void sqlite3CommitInternalChanges(sqlite3 *db){
  db->flags &= ~SQLITE_InternChanges;
}

/*
** Clear the column names from a table or view.
*/
static void sqliteResetColumnNames(Table *pTable){
  int i;
  Column *pCol;
  assert( pTable!=0 );
  if( (pCol = pTable->aCol)!=0 ){
    for(i=0; i<pTable->nCol; i++, pCol++){
      sqliteFree(pCol->zName);
      sqlite3ExprDelete(pCol->pDflt);
      sqliteFree(pCol->zType);
      sqliteFree(pCol->zColl);
    }
    sqliteFree(pTable->aCol);
  }
  pTable->aCol = 0;
  pTable->nCol = 0;
}

/*
** Remove the memory data structures associated with the given
** Table.  No changes are made to disk by this routine.
**
** This routine just deletes the data structure.  It does not unlink
** the table data structure from the hash table.  Nor does it remove
** foreign keys from the sqlite.aFKey hash table.  But it does destroy
** memory structures of the indices and foreign keys associated with 
** the table.
**
** Indices associated with the table are unlinked from the "db"
** data structure if db!=NULL.  If db==NULL, indices attached to
** the table are deleted, but it is assumed they have already been
** unlinked.
*/
void sqlite3DeleteTable(sqlite3 *db, Table *pTable){
}

/*
** Given a token, return a string that consists of the text of that
** token with any quotations removed.  Space to hold the returned string
** is obtained from sqliteMalloc() and must be freed by the calling
** function.
**
** Tokens are often just pointers into the original SQL text and so
** are not \000 terminated and are not persistent.  The returned string
** is \000 terminated and is persistent.
*/
char *sqlite3NameFromToken(Token *pName){
  char *zName;
  if( pName ){
    zName = sqliteStrNDup((char*)pName->z, pName->n);
    sqlite3Dequote(zName);
  }else{
    zName = 0;
  }
  return zName;
}

/*
** Begin constructing a new table representation in memory.  This is
** the first of several action routines that get called in response
** to a CREATE TABLE statement.  In particular, this routine is called
** after seeing tokens "CREATE" and "TABLE" and the table name. The isTemp
** flag is true if the table should be stored in the auxiliary database
** file instead of in the main database file.  This is normally the case
** when the "TEMP" or "TEMPORARY" keyword occurs in between
** CREATE and TABLE.
**
** The new table record is initialized and put in pParse->pNewTable.
** As more of the CREATE TABLE statement is parsed, additional action
** routines will be called to add more information to this record.
** At the end of the CREATE TABLE statement, the sqlite3EndTable() routine
** is called to complete the construction of the new table record.
*/
void sqlite3StartTable(
  Parse *pParse,   /* Parser context */
  Token *pName1,   /* First part of the name of the table or view */
  Token *pName2,   /* Second part of the name of the table or view */
  int isTemp,      /* True if this is a TEMP table */
  int isView,      /* True if this is a VIEW */
  int noErr        /* Do nothing if table already exists */
){
    ParsedResultItem item;
    item.sqltype = SQLTYPE_CREATE_TABLE;
    sqlite3ParsedResultArrayAppend(&pParse->parsed, &item);

}

/*
** This macro is used to compare two strings in a case-insensitive manner.
** It is slightly faster than calling sqlite3StrICmp() directly, but
** produces larger code.
**
** WARNING: This macro is not compatible with the strcmp() family. It
** returns true if the two strings are equal, otherwise false.
*/
#define STRICMP(x, y) (\
sqlite3UpperToLower[*(unsigned char *)(x)]==   \
sqlite3UpperToLower[*(unsigned char *)(y)]     \
&& sqlite3StrICmp((x)+1,(y)+1)==0 )

/*
** Add a new column to the table currently being constructed.
**
** The parser calls this routine once for each column declaration
** in a CREATE TABLE statement.  sqlite3StartTable() gets called
** first to get things going.  Then this routine is called for each
** column.
*/
void sqlite3AddColumn(Parse *pParse, Token *pName){

}

/*
** This routine is called by the parser while in the middle of
** parsing a CREATE TABLE statement.  A "NOT NULL" constraint has
** been seen on a column.  This routine sets the notNull flag on
** the column currently under construction.
*/
void sqlite3AddNotNull(Parse *pParse, int onError){
  /* Table *p; */
  /* int i; */
  /* if( (p = pParse->pNewTable)==0 ) return; */
  /* i = p->nCol-1; */
  /* if( i>=0 ) p->aCol[i].notNull = onError; */
}

/*
** Scan the column type name zType (length nType) and return the
** associated affinity type.
**
** This routine does a case-independent search of zType for the 
** substrings in the following table. If one of the substrings is
** found, the corresponding affinity is returned. If zType contains
** more than one of the substrings, entries toward the top of 
** the table take priority. For example, if zType is 'BLOBINT', 
** SQLITE_AFF_INTEGER is returned.
**
** Substring     | Affinity
** --------------------------------
** 'INT'         | SQLITE_AFF_INTEGER
** 'CHAR'        | SQLITE_AFF_TEXT
** 'CLOB'        | SQLITE_AFF_TEXT
** 'TEXT'        | SQLITE_AFF_TEXT
** 'BLOB'        | SQLITE_AFF_NONE
** 'REAL'        | SQLITE_AFF_REAL
** 'FLOA'        | SQLITE_AFF_REAL
** 'DOUB'        | SQLITE_AFF_REAL
**
** If none of the substrings in the above table are found,
** SQLITE_AFF_NUMERIC is returned.
*/
char sqlite3AffinityType(const Token *pType){
  u32 h = 0;
  char aff = SQLITE_AFF_NUMERIC;
  const unsigned char *zIn = pType->z;
  const unsigned char *zEnd = &pType->z[pType->n];

  while( zIn!=zEnd ){
    h = (h<<8) + sqlite3UpperToLower[*zIn];
    zIn++;
    if( h==(('c'<<24)+('h'<<16)+('a'<<8)+'r') ){             /* CHAR */
      aff = SQLITE_AFF_TEXT; 
    }else if( h==(('c'<<24)+('l'<<16)+('o'<<8)+'b') ){       /* CLOB */
      aff = SQLITE_AFF_TEXT;
    }else if( h==(('t'<<24)+('e'<<16)+('x'<<8)+'t') ){       /* TEXT */
      aff = SQLITE_AFF_TEXT;
    }else if( h==(('b'<<24)+('l'<<16)+('o'<<8)+'b')          /* BLOB */
        && (aff==SQLITE_AFF_NUMERIC || aff==SQLITE_AFF_REAL) ){
      aff = SQLITE_AFF_NONE;
#ifndef SQLITE_OMIT_FLOATING_POINT
    }else if( h==(('r'<<24)+('e'<<16)+('a'<<8)+'l')          /* REAL */
        && aff==SQLITE_AFF_NUMERIC ){
      aff = SQLITE_AFF_REAL;
    }else if( h==(('f'<<24)+('l'<<16)+('o'<<8)+'a')          /* FLOA */
        && aff==SQLITE_AFF_NUMERIC ){
      aff = SQLITE_AFF_REAL;
    }else if( h==(('d'<<24)+('o'<<16)+('u'<<8)+'b')          /* DOUB */
        && aff==SQLITE_AFF_NUMERIC ){
      aff = SQLITE_AFF_REAL;
#endif
    }else if( (h&0x00FFFFFF)==(('i'<<16)+('n'<<8)+'t') ){    /* INT */
      aff = SQLITE_AFF_INTEGER;
      break;
    }
  }

  return aff;
}

/*
** This routine is called by the parser while in the middle of
** parsing a CREATE TABLE statement.  The pFirst token is the first
** token in the sequence of tokens that describe the type of the
** column currently under construction.   pLast is the last token
** in the sequence.  Use this information to construct a string
** that contains the typename of the column and store that string
** in zType.
*/ 
void sqlite3AddColumnType(Parse *pParse, Token *pType){

}

/*
** The expression is the default value for the most recently added column
** of the table currently under construction.
**
** Default value expressions must be constant.  Raise an exception if this
** is not the case.
**
** This routine is called by the parser while in the middle of
** parsing a CREATE TABLE statement.
*/
void sqlite3AddDefaultValue(Parse *pParse, Expr *pExpr){

}

/*
** Designate the PRIMARY KEY for the table.  pList is a list of names 
** of columns that form the primary key.  If pList is NULL, then the
** most recently added column of the table is the primary key.
**
** A table can have at most one primary key.  If the table already has
** a primary key (and this is the second primary key) then create an
** error.
**
** If the PRIMARY KEY is on a single column whose datatype is INTEGER,
** then we will try to use that column as the rowid.  Set the Table.iPKey
** field of the table under construction to be the index of the
** INTEGER PRIMARY KEY column.  Table.iPKey is set to -1 if there is
** no INTEGER PRIMARY KEY.
**
** If the key is not an INTEGER PRIMARY KEY, then create a unique
** index for the key.  No index is created for INTEGER PRIMARY KEYs.
*/
void sqlite3AddPrimaryKey(
  Parse *pParse,    /* Parsing context */
  ExprList *pList,  /* List of field names to be indexed */
  int onError,      /* What to do with a uniqueness conflict */
  int autoInc,      /* True if the AUTOINCREMENT keyword is present */
  int sortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){

}

/*
** Add a new CHECK constraint to the table currently under construction.
*/
void sqlite3AddCheckConstraint(
  Parse *pParse,    /* Parsing context */
  Expr *pCheckExpr  /* The check expression */
){

}

/*
** Set the collation function of the most recently parsed table column
** to the CollSeq given.
*/
void sqlite3AddCollateType(Parse *pParse, const char *zType, int nType){

}

/*
** This function returns the collation sequence for database native text
** encoding identified by the string zName, length nName.
**
** If the requested collation sequence is not available, or not available
** in the database native encoding, the collation factory is invoked to
** request it. If the collation factory does not supply such a sequence,
** and the sequence is available in another text encoding, then that is
** returned instead.
**
** If no versions of the requested collations sequence are available, or
** another error occurs, NULL is returned and an error message written into
** pParse.
*/
CollSeq *sqlite3LocateCollSeq(Parse *pParse, const char *zName, int nName){

}

static int identLength(const char *z){
  int n;
  for(n=0; *z; n++, z++){
    if( *z=='"' ){ n++; }
  }
  return n + 2;
}

/*
** Write an identifier onto the end of the given string.  Add
** quote characters as needed.
*/
static void identPut(char *z, int *pIdx, char *zSignedIdent){
  unsigned char *zIdent = (unsigned char*)zSignedIdent;
  int i, j, needQuote;
  i = *pIdx;
  for(j=0; zIdent[j]; j++){
    if( !isalnum(zIdent[j]) && zIdent[j]!='_' ) break;
  }
  needQuote =  zIdent[j]!=0 || isdigit(zIdent[0])
                  || sqlite3KeywordCode(zIdent, j)!=TK_ID;
  if( needQuote ) z[i++] = '"';
  for(j=0; zIdent[j]; j++){
    z[i++] = zIdent[j];
    if( zIdent[j]=='"' ) z[i++] = '"';
  }
  if( needQuote ) z[i++] = '"';
  z[i] = 0;
  *pIdx = i;
}

/*
** Generate a CREATE TABLE statement appropriate for the given
** table.  Memory to hold the text of the statement is obtained
** from sqliteMalloc() and must be freed by the calling function.
*/
static char *createTableStmt(Table *p, int isTemp){
  int i, k, n;
  char *zStmt;
  char *zSep, *zSep2, *zEnd, *z;
  Column *pCol;
  n = 0;
  for(pCol = p->aCol, i=0; i<p->nCol; i++, pCol++){
    n += identLength(pCol->zName);
    z = pCol->zType;
    if( z ){
      n += (strlen(z) + 1);
    }
  }
  n += identLength(p->zName);
  if( n<50 ){
    zSep = "";
    zSep2 = ",";
    zEnd = ")";
  }else{
    zSep = "\n  ";
    zSep2 = ",\n  ";
    zEnd = "\n)";
  }
  n += 35 + 6*p->nCol;
  zStmt = sqliteMallocRaw( n );
  if( zStmt==0 ) return 0;
  strcpy(zStmt, !OMIT_TEMPDB&&isTemp ? "CREATE TEMP TABLE ":"CREATE TABLE ");
  k = strlen(zStmt);
  identPut(zStmt, &k, p->zName);
  zStmt[k++] = '(';
  for(pCol=p->aCol, i=0; i<p->nCol; i++, pCol++){
    strcpy(&zStmt[k], zSep);
    k += strlen(&zStmt[k]);
    zSep = zSep2;
    identPut(zStmt, &k, pCol->zName);
    if( (z = pCol->zType)!=0 ){
      zStmt[k++] = ' ';
      strcpy(&zStmt[k], z);
      k += strlen(z);
    }
  }
  strcpy(&zStmt[k], zEnd);
  return zStmt;
}

void sqlite3EndTable(
  Parse *pParse,          /* Parse context */
  Token *pCons,           /* The ',' token after the last column defn. */
  Token *pEnd,            /* The final ')' token in the CREATE TABLE */
  Select *pSelect         /* Select from a "CREATE ... AS SELECT" */
){

}

#ifndef SQLITE_OMIT_VIEW
/*
** The parser calls this routine in order to create a new VIEW
*/
void sqlite3CreateView(
  Parse *pParse,     /* The parsing context */
  Token *pBegin,     /* The CREATE token that begins the statement */
  Token *pName1,     /* The token that holds the name of the view */
  Token *pName2,     /* The token that holds the name of the view */
  Select *pSelect,   /* A SELECT statement that will become the new view */
  int isTemp         /* TRUE for a TEMPORARY view */
){

}
#endif /* SQLITE_OMIT_VIEW */

#ifndef SQLITE_OMIT_VIEW

#endif /* SQLITE_OMIT_VIEW */

#ifndef SQLITE_OMIT_VIEW

#else
# define sqliteViewResetAll(A,B)
#endif /* SQLITE_OMIT_VIEW */

#ifndef SQLITE_OMIT_AUTOVACUUM

#endif

/*
** This routine is called to do the work of a DROP TABLE statement.
** pName is the name of the table to be dropped.
*/
void sqlite3DropTable(Parse *pParse, SrcList *pName, int isView, int noErr){
    ParsedResultItem item;
    item.sqltype = SQLTYPE_DROP_TABLE;
    sqlite3ParsedResultArrayAppend(&pParse->parsed, &item);
    sqlite3SrcListDelete(pName);
}

void sqlite3CreateForeignKey(
  Parse *pParse,       /* Parsing context */
  ExprList *pFromCol,  /* Columns in this table that point to other table */
  Token *pTo,          /* Name of the other table */
  ExprList *pToCol,    /* Columns in the other table */
  int flags            /* Conflict resolution algorithms. */
){
}

/*
** This routine is called when an INITIALLY IMMEDIATE or INITIALLY DEFERRED
** clause is seen as part of a foreign key definition.  The isDeferred
** parameter is 1 for INITIALLY DEFERRED and 0 for INITIALLY IMMEDIATE.
** The behavior of the most recently created foreign key is adjusted
** accordingly.
*/
void sqlite3DeferForeignKey(Parse *pParse, int isDeferred){

}

void sqlite3CreateIndex(
  Parse *pParse,     /* All information about this parse */
  Token *pName1,     /* First part of index name. May be NULL */
  Token *pName2,     /* Second part of index name. May be NULL */
  SrcList *pTblName, /* Table to index. Use pParse->pNewTable if 0 */
  ExprList *pList,   /* A list of columns to be indexed */
  int onError,       /* OE_Abort, OE_Ignore, OE_Replace, or OE_None */
  Token *pStart,     /* The CREATE token that begins a CREATE TABLE statement */
  Token *pEnd,       /* The ")" that closes the CREATE INDEX statement */
  int sortOrder,     /* Sort order of primary key when pList==NULL */
  int ifNotExist     /* Omit error if index already exists */
){
    sqlite3SrcListDelete(pTblName);
    sqlite3ExprListDelete(pList);
}

void sqlite3DefaultRowEst(Index *pIdx){
  unsigned *a = pIdx->aiRowEst;
  int i;
  assert( a!=0 );
  a[0] = 1000000;
  for(i=pIdx->nColumn; i>=1; i--){
    a[i] = 10;
  }
  if( pIdx->onError!=OE_None ){
    a[pIdx->nColumn] = 1;
  }
}

/*
** This routine will drop an existing named index.  This routine
** implements the DROP INDEX statement.
*/
void sqlite3DropIndex(Parse *pParse, SrcList *pName, int ifExists){

}

int sqlite3ArrayAllocate(void **ppArray, int szEntry, int initSize){
  char *p;
  int *an = (int*)&ppArray[1];
  if( an[0]>=an[1] ){
    void *pNew;
    int newSize;
    newSize = an[1]*2 + initSize;
    pNew = sqliteRealloc(*ppArray, newSize*szEntry);
    if( pNew==0 ){
      return -1;
    }
    an[1] = newSize;
    *ppArray = pNew;
  }
  p = *ppArray;
  memset(&p[an[0]*szEntry], 0, szEntry);
  return an[0]++;
}

/*
** Append a new element to the given IdList.  Create a new IdList if
** need be.
**
** A new IdList is returned, or NULL if malloc() fails.
*/
IdList *sqlite3IdListAppend(IdList *pList, Token *pToken){
  int i;
  if( pList==0 ){
    pList = sqliteMalloc( sizeof(IdList) );
    if( pList==0 ) return 0;
    pList->nAlloc = 0;
  }
  i = sqlite3ArrayAllocate((void**)&pList->a, sizeof(pList->a[0]), 5);
  if( i<0 ){
    sqlite3IdListDelete(pList);
    return 0;
  }
  pList->a[i].zName = sqlite3NameFromToken(pToken);
  return pList;
}

/*
** Delete an IdList.
*/
void sqlite3IdListDelete(IdList *pList){
  int i;
  if( pList==0 ) return;
  for(i=0; i<pList->nId; i++){
    sqliteFree(pList->a[i].zName);
  }
  sqliteFree(pList->a);
  sqliteFree(pList);
}

/*
** Return the index in pList of the identifier named zId.  Return -1
** if not found.
*/
int sqlite3IdListIndex(IdList *pList, const char *zName){
  int i;
  if( pList==0 ) return -1;
  for(i=0; i<pList->nId; i++){
    if( sqlite3StrICmp(pList->a[i].zName, zName)==0 ) return i;
  }
  return -1;
}

SrcList *sqlite3SrcListAppend(SrcList *pList, Token *pTable, Token *pDatabase){
  struct SrcList_item *pItem;
  if( pList==0 ){
    pList = sqliteMalloc( sizeof(SrcList) );
    if( pList==0 ) return 0;
    pList->nAlloc = 1;
  }
  if( pList->nSrc>=pList->nAlloc ){
    SrcList *pNew;
    pList->nAlloc *= 2;
    pNew = sqliteRealloc(pList,
               sizeof(*pList) + (pList->nAlloc-1)*sizeof(pList->a[0]) );
    if( pNew==0 ){
      sqlite3SrcListDelete(pList);
      return 0;
    }
    pList = pNew;
  }
  pItem = &pList->a[pList->nSrc];
  memset(pItem, 0, sizeof(pList->a[0]));
  if( pDatabase && pDatabase->z==0 ){
    pDatabase = 0;
  }
  if( pDatabase && pTable ){
    Token *pTemp = pDatabase;
    pDatabase = pTable;
    pTable = pTemp;
  }
  pItem->zName = sqlite3NameFromToken(pTable);
  pItem->zDatabase = sqlite3NameFromToken(pDatabase);

  if (pTable) { 
      pItem->tableToken = *pTable;
  }

  if (pDatabase) {
       pItem->dbToken = *pDatabase;
  }

  pItem->iCursor = -1;
  pItem->isPopulated = 0;
  pList->nSrc++;
  return pList;
}

/*
** Assign cursors to all tables in a SrcList
*/
void sqlite3SrcListAssignCursors(Parse *pParse, SrcList *pList){
  int i;
  struct SrcList_item *pItem;
  assert(pList || sqlite3MallocFailed() );
  if( pList ){
    for(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
      if( pItem->iCursor>=0 ) break;
      pItem->iCursor = pParse->nTab++;
      if( pItem->pSelect ){
        sqlite3SrcListAssignCursors(pParse, pItem->pSelect->pSrc);
      }
    }
  }
}

/*
** Add an alias to the last identifier on the given identifier list.
*/
void sqlite3SrcListAddAlias(SrcList *pList, Token *pToken){
  if( pList && pList->nSrc>0 ){
    pList->a[pList->nSrc-1].zAlias = sqlite3NameFromToken(pToken);
  }
}

/*
** Delete an entire SrcList including all its substructure.
*/
void sqlite3SrcListDelete(SrcList *pList){
  int i;
  struct SrcList_item *pItem;
  if( pList==0 ) return;
  for(pItem=pList->a, i=0; i<pList->nSrc; i++, pItem++){
    sqliteFree(pItem->zDatabase);
    sqliteFree(pItem->zName);
    sqliteFree(pItem->zAlias);
    //sqlite3DeleteTable(0, pItem->pTab);
    sqlite3SelectDelete(pItem->pSelect);
    sqlite3ExprDelete(pItem->pOn);
    sqlite3IdListDelete(pItem->pUsing);
  }
  sqliteFree(pList);
}

/*
** Begin a transaction
*/
void sqlite3BeginTransaction(Parse *pParse, int type){
    ParsedResultItem item;
    item.sqltype = type;
    sqlite3ParsedResultArrayAppend(&pParse->parsed, &item);
}

/*
** Commit a transaction
*/
void sqlite3CommitTransaction(Parse *pParse){
    ParsedResultItem item;
    item.sqltype = SQLTYPE_TRANSACTION_COMMIT;
    sqlite3ParsedResultArrayAppend(&pParse->parsed, &item);
}

/*
** Rollback a transaction
*/
void sqlite3RollbackTransaction(Parse *pParse){
    ParsedResultItem item;
    item.sqltype = SQLTYPE_TRANSACTION_ROLLBACK;
    sqlite3ParsedResultArrayAppend(&pParse->parsed, &item);
}

/*
** Check to see if pIndex uses the collating sequence pColl.  Return
** true if it does and false if it does not.
*/
#ifndef SQLITE_OMIT_REINDEX
static int collationMatch(const char *zColl, Index *pIndex){
  int i;
  for(i=0; i<pIndex->nColumn; i++){
    const char *z = pIndex->azColl[i];
    if( z==zColl || (z && zColl && 0==sqlite3StrICmp(z, zColl)) ){
      return 1;
    }
  }
  return 0;
}
#endif

/*
** Recompute all indices of pTab that use the collating sequence pColl.
** If pColl==0 then recompute all indices of pTab.
*/
#ifndef SQLITE_OMIT_REINDEX

#endif

/*
** Recompute all indices of all tables in all databases where the
** indices use the collating sequence pColl.  If pColl==0 then recompute
** all indices everywhere.
*/
#ifndef SQLITE_OMIT_REINDEX

#endif

/*
** Generate code for the REINDEX command.
**
**        REINDEX                            -- 1
**        REINDEX  <collation>               -- 2
**        REINDEX  ?<database>.?<tablename>  -- 3
**        REINDEX  ?<database>.?<indexname>  -- 4
**
** Form 1 causes all indices in all attached databases to be rebuilt.
** Form 2 rebuilds all indices in all databases that use the named
** collating function.  Forms 3 and 4 rebuild the named index or all
** indices associated with the named table.
*/
#ifndef SQLITE_OMIT_REINDEX

#endif

/*
** Return a dynamicly allocated KeyInfo structure that can be used
** with OP_OpenRead or OP_OpenWrite to access database index pIdx.
**
** If successful, a pointer to the new structure is returned. In this case
** the caller is responsible for calling sqliteFree() on the returned 
** pointer. If an error occurs (out of memory or missing collation 
** sequence), NULL is returned and the state of pParse updated to reflect
** the error.
*/
KeyInfo *sqlite3IndexKeyinfo(Parse *pParse, Index *pIdx){
  int i;
  int nCol = pIdx->nColumn;
  int nBytes = sizeof(KeyInfo) + (nCol-1)*sizeof(CollSeq*) + nCol;
  KeyInfo *pKey = (KeyInfo *)sqliteMalloc(nBytes);

  if( pKey ){
    pKey->aSortOrder = (u8 *)&(pKey->aColl[nCol]);
    assert( &pKey->aSortOrder[nCol]==&(((u8 *)pKey)[nBytes]) );
    for(i=0; i<nCol; i++){
      char *zColl = pIdx->azColl[i];
      assert( zColl );
      pKey->aColl[i] = sqlite3LocateCollSeq(pParse, zColl, -1);
      pKey->aSortOrder[i] = pIdx->aSortOrder[i];
    }
    pKey->nField = nCol;
  }

  if( pParse->nErr ){
    sqliteFree(pKey);
    pKey = 0;
  }
  return pKey;
}

SetStatement* sqlite3SetStatementNew(ExprList *pList, Token *pValue) {
    SetStatement *pNew = NULL;   
    pNew = (SetStatement*) sqliteMalloc(sizeof(*pNew));
    if (pNew == NULL) {
        return NULL;
    }

    pNew->pSetList = pList;
    if (pValue) { pNew->value = *pValue; }
    return pNew;
}

void sqlite3SetStatementDelete(SetStatement *setObj) {
    if (setObj == NULL) return;

    sqlite3ExprListDelete(setObj->pSetList);
    sqliteFree(setObj);
}

void sqlite3SetStatement(Parse *pParse, ExprList *pExprList, Token *pToken, SqlType sqltype) {
    if (pParse->nErr > 0) { 
        sqlite3ExprListDelete(pExprList);
        return; 
    }
    SetStatement *setObj = sqlite3SetStatementNew(pExprList, pToken);   
    if (setObj == NULL) {
        sqlite3ErrorMsg(pParse, "sqlite3SetStatementNew return NULL, may the malloc failed!");
    }

    ParsedResultItem item;
    item.sqltype = sqltype;
    item.result.setObj = setObj;
    sqlite3ParsedResultArrayAppend(&pParse->parsed, &item);
}

void sqlite3CheckSetScope(Parse *pParse, Token *pScope) {
    if (pScope == NULL) return;
    if ( strncmp(pScope->z, "@@local", pScope->n) != 0 && strncmp(pScope->z, "@@global", pScope->n) != 0 &&
            strncmp(pScope->z, "@@session", pScope->n) != 0 ) {
       sqlite3ErrorMsg(pParse, "illegal set statement scope qualifier, near \"%T\" ", pScope); 
    } 
}

void sqlite3ShowStatement(Parse *pParse, ShowStatementType showtype) {
    ParsedResultItem item;
    item.sqltype = SQLTYPE_SHOW;
    item.result.showType = showtype;
    sqlite3ParsedResultArrayAppend(&pParse->parsed, &item);
}
/*******************************file from trigger.c**************************************/
#ifndef SQLITE_OMIT_TRIGGER
/*
** Delete a linked list of TriggerStep structures.
*/
void sqlite3DeleteTriggerStep(TriggerStep *pTriggerStep){
  while( pTriggerStep ){
    TriggerStep * pTmp = pTriggerStep;
    pTriggerStep = pTriggerStep->pNext;

    if( pTmp->target.dyn ) sqliteFree((char*)pTmp->target.z);
    sqlite3ExprDelete(pTmp->pWhere);
    sqlite3ExprListDelete(pTmp->pExprList);
    sqlite3SelectDelete(pTmp->pSelect);
    sqlite3IdListDelete(pTmp->pIdList);

    sqliteFree(pTmp);
  }
}

/*
** This is called by the parser when it sees a CREATE TRIGGER statement
** up to the point of the BEGIN before the trigger actions.  A Trigger
** structure is generated based on the information available and stored
** in pParse->pNewTrigger.  After the trigger actions have been parsed, the
** sqlite3FinishTrigger() function is called to complete the trigger
** construction process.
*/
void sqlite3BeginTrigger(
  Parse *pParse,      /* The parse context of the CREATE TRIGGER statement */
  Token *pName1,      /* The name of the trigger */
  Token *pName2,      /* The name of the trigger */
  int tr_tm,          /* One of TK_BEFORE, TK_AFTER, TK_INSTEAD */
  int op,             /* One of TK_INSERT, TK_UPDATE, TK_DELETE */
  IdList *pColumns,   /* column list if this is an UPDATE OF trigger */
  SrcList *pTableName,/* The name of the table/view the trigger applies to */
  int foreach,        /* One of TK_ROW or TK_STATEMENT */
  Expr *pWhen,        /* WHEN clause */
  int isTemp          /* True if the TEMPORARY keyword is present */
){

}

/*
** This routine is called after all of the trigger actions have been parsed
** in order to complete the process of building the trigger.
*/
void sqlite3FinishTrigger(
  Parse *pParse,          /* Parser context */
  TriggerStep *pStepList, /* The triggered program */
  Token *pAll             /* Token that describes the complete CREATE TRIGGER */
){

}

/*
** Make a copy of all components of the given trigger step.  This has
** the effect of copying all Expr.token.z values into memory obtained
** from sqliteMalloc().  As initially created, the Expr.token.z values
** all point to the input string that was fed to the parser.  But that
** string is ephemeral - it will go away as soon as the sqlite3_exec()
** call that started the parser exits.  This routine makes a persistent
** copy of all the Expr.token.z strings so that the TriggerStep structure
** will be valid even after the sqlite3_exec() call returns.
*/
static void sqlitePersistTriggerStep(TriggerStep *p){
  if( p->target.z ){
    p->target.z = (u8*)sqliteStrNDup((char*)p->target.z, p->target.n);
    p->target.dyn = 1;
  }
  if( p->pSelect ){
    Select *pNew = sqlite3SelectDup(p->pSelect);
    sqlite3SelectDelete(p->pSelect);
    p->pSelect = pNew;
  }
  if( p->pWhere ){
    Expr *pNew = sqlite3ExprDup(p->pWhere);
    sqlite3ExprDelete(p->pWhere);
    p->pWhere = pNew;
  }
  if( p->pExprList ){
    ExprList *pNew = sqlite3ExprListDup(p->pExprList);
    sqlite3ExprListDelete(p->pExprList);
    p->pExprList = pNew;
  }
  if( p->pIdList ){
    IdList *pNew = sqlite3IdListDup(p->pIdList);
    sqlite3IdListDelete(p->pIdList);
    p->pIdList = pNew;
  }
}

/*
** Turn a SELECT statement (that the pSelect parameter points to) into
** a trigger step.  Return a pointer to a TriggerStep structure.
**
** The parser calls this routine when it finds a SELECT statement in
** body of a TRIGGER.  
*/
TriggerStep *sqlite3TriggerSelectStep(Select *pSelect){
  TriggerStep *pTriggerStep = sqliteMalloc(sizeof(TriggerStep));
  if( pTriggerStep==0 ) {
    sqlite3SelectDelete(pSelect);
    return 0;
  }

  pTriggerStep->op = TK_SELECT;
  pTriggerStep->pSelect = pSelect;
  pTriggerStep->orconf = OE_Default;
  sqlitePersistTriggerStep(pTriggerStep);

  return pTriggerStep;
}

/*
** Build a trigger step out of an INSERT statement.  Return a pointer
** to the new trigger step.
**
** The parser calls this routine when it sees an INSERT inside the
** body of a trigger.
*/
TriggerStep *sqlite3TriggerInsertStep(
  Token *pTableName,  /* Name of the table into which we insert */
  IdList *pColumn,    /* List of columns in pTableName to insert into */
  ExprList *pEList,   /* The VALUE clause: a list of values to be inserted */
  Select *pSelect,    /* A SELECT statement that supplies values */
  int orconf          /* The conflict algorithm (OE_Abort, OE_Replace, etc.) */
){
  TriggerStep *pTriggerStep = sqliteMalloc(sizeof(TriggerStep));

  assert(pEList == 0 || pSelect == 0);
  assert(pEList != 0 || pSelect != 0);

  if( pTriggerStep ){
    pTriggerStep->op = TK_INSERT;
    pTriggerStep->pSelect = pSelect;
    pTriggerStep->target  = *pTableName;
    pTriggerStep->pIdList = pColumn;
    pTriggerStep->pExprList = pEList;
    pTriggerStep->orconf = orconf;
    sqlitePersistTriggerStep(pTriggerStep);
  }else{
    sqlite3IdListDelete(pColumn);
    sqlite3ExprListDelete(pEList);
    sqlite3SelectDup(pSelect);
  }

  return pTriggerStep;
}

/*
** Construct a trigger step that implements an UPDATE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees an UPDATE statement inside the body of a CREATE TRIGGER.
*/
TriggerStep *sqlite3TriggerUpdateStep(
  Token *pTableName,   /* Name of the table to be updated */
  ExprList *pEList,    /* The SET clause: list of column and new values */
  Expr *pWhere,        /* The WHERE clause */
  int orconf           /* The conflict algorithm. (OE_Abort, OE_Ignore, etc) */
){
  TriggerStep *pTriggerStep = sqliteMalloc(sizeof(TriggerStep));
  if( pTriggerStep==0 ) return 0;

  pTriggerStep->op = TK_UPDATE;
  pTriggerStep->target  = *pTableName;
  pTriggerStep->pExprList = pEList;
  pTriggerStep->pWhere = pWhere;
  pTriggerStep->orconf = orconf;
  sqlitePersistTriggerStep(pTriggerStep);

  return pTriggerStep;
}

/*
** Construct a trigger step that implements a DELETE statement and return
** a pointer to that trigger step.  The parser calls this routine when it
** sees a DELETE statement inside the body of a CREATE TRIGGER.
*/
TriggerStep *sqlite3TriggerDeleteStep(Token *pTableName, Expr *pWhere){
  TriggerStep *pTriggerStep = sqliteMalloc(sizeof(TriggerStep));
  if( pTriggerStep==0 ) return 0;

  pTriggerStep->op = TK_DELETE;
  pTriggerStep->target  = *pTableName;
  pTriggerStep->pWhere = pWhere;
  pTriggerStep->orconf = OE_Default;
  sqlitePersistTriggerStep(pTriggerStep);

  return pTriggerStep;
}

/* 
** Recursively delete a Trigger structure
*/
void sqlite3DeleteTrigger(Trigger *pTrigger){
  if( pTrigger==0 ) return;
  sqlite3DeleteTriggerStep(pTrigger->step_list);
  sqliteFree(pTrigger->name);
  sqliteFree(pTrigger->table);
  sqlite3ExprDelete(pTrigger->pWhen);
  sqlite3IdListDelete(pTrigger->pColumns);
  if( pTrigger->nameToken.dyn ) sqliteFree((char*)pTrigger->nameToken.z);
  sqliteFree(pTrigger);
}

/*
** This function is called to drop a trigger from the database schema. 
**
** This may be called directly from the parser and therefore identifies
** the trigger by name.  The sqlite3DropTriggerPtr() routine does the
** same job as this routine except it takes a pointer to the trigger
** instead of the trigger name.
**/
void sqlite3DropTrigger(Parse *pParse, SrcList *pName){

}

/*
** Drop a trigger given a pointer to that trigger. 
*/
void sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger){

}

static int checkColumnOverLap(IdList *pIdList, ExprList *pEList){
  int e;
  if( !pIdList || !pEList ) return 1;
  for(e=0; e<pEList->nExpr; e++){
    if( sqlite3IdListIndex(pIdList, pEList->a[e].zName)>=0 ) return 1;
  }
  return 0; 
}

/*
** Return a bit vector to indicate what kind of triggers exist for operation
** "op" on table pTab.  If pChanges is not NULL then it is a list of columns
** that are being updated.  Triggers only match if the ON clause of the
** trigger definition overlaps the set of columns being updated.
**
** The returned bit vector is some combination of TRIGGER_BEFORE and
** TRIGGER_AFTER.
*/
int sqlite3TriggersExist(
  Parse *pParse,          /* Used to check for recursive triggers */
  Table *pTab,            /* The table the contains the triggers */
  int op,                 /* one of TK_DELETE, TK_INSERT, TK_UPDATE */
  ExprList *pChanges      /* Columns that change in an UPDATE statement */
){
  Trigger *pTrigger = pTab->pTrigger;
  int mask = 0;

  while( pTrigger ){
    if( pTrigger->op==op && checkColumnOverLap(pTrigger->pColumns, pChanges) ){
      mask |= pTrigger->tr_tm;
    }
    pTrigger = pTrigger->pNext;
  }
  return mask;
}
#endif /* !defined(SQLITE_OMIT_TRIGGER) */
/***********************************file from alter.c*************************/
/*
** The code in this file only exists if we are not omitting the
** ALTER TABLE logic from the build.
*/
#ifndef SQLITE_OMIT_ALTERTABLE

#ifndef SQLITE_OMIT_TRIGGER

#endif   /* !SQLITE_OMIT_TRIGGER */

void sqlite3AlterRenameTable(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){

}


/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
void sqlite3AlterFinishAddColumn(Parse *pParse, Token *pColDef){

}

void sqlite3AlterBeginAddColumn(Parse *pParse, SrcList *pSrc){

}
#endif  /* SQLITE_ALTER_TABLE */
/************************************file from os_unix.c********************************/
#if OS_UNIX              /* This file is used on unix only */

/*
** These #defines should enable >2GB file support on Posix if the
** underlying operating system supports it.  If the OS lacks
** large file support, these should be no-ops.
**
** Large file support can be disabled using the -DSQLITE_DISABLE_LFS switch
** on the compiler command line.  This is necessary if you are compiling
** on a recent machine (ex: RedHat 7.2) but you want your code to work
** on an older machine (ex: RedHat 6.0).  If you compile on RedHat 7.2
** without this option, LFS is enable.  But LFS does not exist in the kernel
** in RedHat 6.0, so the code won't work.  Hence, for maximum binary
** portability you should omit LFS.
*/
#ifndef SQLITE_DISABLE_LFS
# define _LARGE_FILE       1
# ifndef _FILE_OFFSET_BITS
#   define _FILE_OFFSET_BITS 64
# endif
# define _LARGEFILE_SOURCE 1
#endif

/*
** standard include files.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

/*
** If we are to be thread-safe, include the pthreads header and define
** the SQLITE_UNIX_THREADS macro.
*/
#if defined(THREADSAFE) && THREADSAFE
# include <pthread.h>
# define SQLITE_UNIX_THREADS 1
#endif

/*
** Default permissions when creating a new file
*/
#ifndef SQLITE_DEFAULT_FILE_PERMISSIONS
# define SQLITE_DEFAULT_FILE_PERMISSIONS 0644
#endif



/*
** The unixFile structure is subclass of OsFile specific for the unix
** protability layer.
*/
typedef struct unixFile unixFile;
struct unixFile {
  IoMethod const *pMethod;  /* Always the first entry */
  struct openCnt *pOpen;    /* Info about all open fd's on this inode */
  struct lockInfo *pLock;   /* Info about locks on this inode */
  int h;                    /* The file descriptor */
  unsigned char locktype;   /* The type of lock held on this fd */
  unsigned char isOpen;     /* True if needs to be closed */
  unsigned char fullSync;   /* Use F_FULLSYNC if available */
  int dirfd;                /* File descriptor for the directory */
  i64 offset;               /* Seek offset */
#ifdef SQLITE_UNIX_THREADS
  pthread_t tid;            /* The thread that "owns" this OsFile */
#endif
};

/*
** Provide the ability to override some OS-layer functions during
** testing.  This is used to simulate OS crashes to verify that 
** commits are atomic even in the event of an OS crash.
*/
#ifdef SQLITE_CRASH_TEST
  extern int sqlite3CrashTestEnable;
  extern int sqlite3CrashOpenReadWrite(const char*, OsFile**, int*);
  extern int sqlite3CrashOpenExclusive(const char*, OsFile**, int);
  extern int sqlite3CrashOpenReadOnly(const char*, OsFile**, int);
# define CRASH_TEST_OVERRIDE(X,A,B,C) \
    if(sqlite3CrashTestEnable){ return X(A,B,C); }
#else
# define CRASH_TEST_OVERRIDE(X,A,B,C)  /* no-op */
#endif


/*
** Include code that is common to all os_*.c files
*/

/*
** Do not include any of the File I/O interface procedures if the
** SQLITE_OMIT_DISKIO macro is defined (indicating that the database
** will be in-memory only)
*/
#ifndef SQLITE_OMIT_DISKIO

/*
** Define various macros that are missing from some systems.
*/
#ifndef O_LARGEFILE
# define O_LARGEFILE 0
#endif
#ifdef SQLITE_DISABLE_LFS
# undef O_LARGEFILE
# define O_LARGEFILE 0
#endif
#ifndef O_NOFOLLOW
# define O_NOFOLLOW 0
#endif
#ifndef O_BINARY
# define O_BINARY 0
#endif

/*
** The DJGPP compiler environment looks mostly like Unix, but it
** lacks the fcntl() system call.  So redefine fcntl() to be something
** that always succeeds.  This means that locking does not occur under
** DJGPP.  But it's DOS - what did you expect?
*/
#ifdef __DJGPP__
# define fcntl(A,B,C) 0
#endif

/*
** The threadid macro resolves to the thread-id or to 0.  Used for
** testing and debugging only.
*/
#ifdef SQLITE_UNIX_THREADS
#define threadid pthread_self()
#else
#define threadid 0
#endif

/*
** Set or check the OsFile.tid field.  This field is set when an OsFile
** is first opened.  All subsequent uses of the OsFile verify that the
** same thread is operating on the OsFile.  Some operating systems do
** not allow locks to be overridden by other threads and that restriction
** means that sqlite3* database handles cannot be moved from one thread
** to another.  This logic makes sure a user does not try to do that
** by mistake.
**
** Version 3.3.1 (2006-01-15):  OsFiles can be moved from one thread to
** another as long as we are running on a system that supports threads
** overriding each others locks (which now the most common behavior)
** or if no locks are held.  But the OsFile.pLock field needs to be
** recomputed because its key includes the thread-id.  See the 
** transferOwnership() function below for additional information
*/
#if defined(SQLITE_UNIX_THREADS)
# define SET_THREADID(X)   (X)->tid = pthread_self()
# define CHECK_THREADID(X) (threadsOverrideEachOthersLocks==0 && \
                            !pthread_equal((X)->tid, pthread_self()))
#else
# define SET_THREADID(X)
# define CHECK_THREADID(X) 0
#endif

/*
** Here is the dirt on POSIX advisory locks:  ANSI STD 1003.1 (1996)
** section 6.5.2.2 lines 483 through 490 specify that when a process
** sets or clears a lock, that operation overrides any prior locks set
** by the same process.  It does not explicitly say so, but this implies
** that it overrides locks set by the same process using a different
** file descriptor.  Consider this test case:
**
**       int fd1 = open("./file1", O_RDWR|O_CREAT, 0644);
**       int fd2 = open("./file2", O_RDWR|O_CREAT, 0644);
**
** Suppose ./file1 and ./file2 are really the same file (because
** one is a hard or symbolic link to the other) then if you set
** an exclusive lock on fd1, then try to get an exclusive lock
** on fd2, it works.  I would have expected the second lock to
** fail since there was already a lock on the file due to fd1.
** But not so.  Since both locks came from the same process, the
** second overrides the first, even though they were on different
** file descriptors opened on different file names.
**
** Bummer.  If you ask me, this is broken.  Badly broken.  It means
** that we cannot use POSIX locks to synchronize file access among
** competing threads of the same process.  POSIX locks will work fine
** to synchronize access for threads in separate processes, but not
** threads within the same process.
**
** To work around the problem, SQLite has to manage file locks internally
** on its own.  Whenever a new database is opened, we have to find the
** specific inode of the database file (the inode is determined by the
** st_dev and st_ino fields of the stat structure that fstat() fills in)
** and check for locks already existing on that inode.  When locks are
** created or removed, we have to look at our own internal record of the
** locks to see if another thread has previously set a lock on that same
** inode.
**
** The OsFile structure for POSIX is no longer just an integer file
** descriptor.  It is now a structure that holds the integer file
** descriptor and a pointer to a structure that describes the internal
** locks on the corresponding inode.  There is one locking structure
** per inode, so if the same inode is opened twice, both OsFile structures
** point to the same locking structure.  The locking structure keeps
** a reference count (so we will know when to delete it) and a "cnt"
** field that tells us its internal lock status.  cnt==0 means the
** file is unlocked.  cnt==-1 means the file has an exclusive lock.
** cnt>0 means there are cnt shared locks on the file.
**
** Any attempt to lock or unlock a file first checks the locking
** structure.  The fcntl() system call is only invoked to set a 
** POSIX lock if the internal lock structure transitions between
** a locked and an unlocked state.
**
** 2004-Jan-11:
** More recent discoveries about POSIX advisory locks.  (The more
** I discover, the more I realize the a POSIX advisory locks are
** an abomination.)
**
** If you close a file descriptor that points to a file that has locks,
** all locks on that file that are owned by the current process are
** released.  To work around this problem, each OsFile structure contains
** a pointer to an openCnt structure.  There is one openCnt structure
** per open inode, which means that multiple OsFiles can point to a single
** openCnt.  When an attempt is made to close an OsFile, if there are
** other OsFiles open on the same inode that are holding locks, the call
** to close() the file descriptor is deferred until all of the locks clear.
** The openCnt structure keeps a list of file descriptors that need to
** be closed and that list is walked (and cleared) when the last lock
** clears.
**
** First, under Linux threads, because each thread has a separate
** process ID, lock operations in one thread do not override locks
** to the same file in other threads.  Linux threads behave like
** separate processes in this respect.  But, if you close a file
** descriptor in linux threads, all locks are cleared, even locks
** on other threads and even though the other threads have different
** process IDs.  Linux threads is inconsistent in this respect.
** (I'm beginning to think that linux threads is an abomination too.)
** The consequence of this all is that the hash table for the lockInfo
** structure has to include the process id as part of its key because
** locks in different threads are treated as distinct.  But the 
** openCnt structure should not include the process id in its
** key because close() clears lock on all threads, not just the current
** thread.  Were it not for this goofiness in linux threads, we could
** combine the lockInfo and openCnt structures into a single structure.
**
** 2004-Jun-28:
** On some versions of linux, threads can override each others locks.
** On others not.  Sometimes you can change the behavior on the same
** system by setting the LD_ASSUME_KERNEL environment variable.  The
** POSIX standard is silent as to which behavior is correct, as far
** as I can tell, so other versions of unix might show the same
** inconsistency.  There is no little doubt in my mind that posix
** advisory locks and linux threads are profoundly broken.
**
** To work around the inconsistencies, we have to test at runtime 
** whether or not threads can override each others locks.  This test
** is run once, the first time any lock is attempted.  A static 
** variable is set to record the results of this test for future
** use.
*/

/*
** An instance of the following structure serves as the key used
** to locate a particular lockInfo structure given its inode.
**
** If threads cannot override each others locks, then we set the
** lockKey.tid field to the thread ID.  If threads can override
** each others locks then tid is always set to zero.  tid is omitted
** if we compile without threading support.
*/
struct lockKey {
  dev_t dev;       /* Device number */
  ino_t ino;       /* Inode number */
#ifdef SQLITE_UNIX_THREADS
  pthread_t tid;   /* Thread ID or zero if threads can override each other */
#endif
};

/*
** An instance of the following structure is allocated for each open
** inode on each thread with a different process ID.  (Threads have
** different process IDs on linux, but not on most other unixes.)
**
** A single inode can have multiple file descriptors, so each OsFile
** structure contains a pointer to an instance of this object and this
** object keeps a count of the number of OsFiles pointing to it.
*/
struct lockInfo {
  struct lockKey key;  /* The lookup key */
  int cnt;             /* Number of SHARED locks held */
  int locktype;        /* One of SHARED_LOCK, RESERVED_LOCK etc. */
  int nRef;            /* Number of pointers to this structure */
};

/*
** An instance of the following structure serves as the key used
** to locate a particular openCnt structure given its inode.  This
** is the same as the lockKey except that the thread ID is omitted.
*/
struct openKey {
  dev_t dev;   /* Device number */
  ino_t ino;   /* Inode number */
};

/*
** An instance of the following structure is allocated for each open
** inode.  This structure keeps track of the number of locks on that
** inode.  If a close is attempted against an inode that is holding
** locks, the close is deferred until all locks clear by adding the
** file descriptor to be closed to the pending list.
*/
struct openCnt {
  struct openKey key;   /* The lookup key */
  int nRef;             /* Number of pointers to this structure */
  int nLock;            /* Number of outstanding locks */
  int nPending;         /* Number of pending close() operations */
  int *aPending;        /* Malloced space holding fd's awaiting a close() */
};

/* 
** These hash tables map inodes and file descriptors (really, lockKey and
** openKey structures) into lockInfo and openCnt structures.  Access to 
** these hash tables must be protected by a mutex.
*/
static Hash lockHash = {SQLITE_HASH_BINARY, 0, 0, 0, 
    sqlite3ThreadSafeMalloc, sqlite3ThreadSafeFree, 0, 0};
static Hash openHash = {SQLITE_HASH_BINARY, 0, 0, 0, 
    sqlite3ThreadSafeMalloc, sqlite3ThreadSafeFree, 0, 0};

#ifdef SQLITE_UNIX_THREADS
/*
** This variable records whether or not threads can override each others
** locks.
**
**    0:  No.  Threads cannot override each others locks.
**    1:  Yes.  Threads can override each others locks.
**   -1:  We don't know yet.
**
** On some systems, we know at compile-time if threads can override each
** others locks.  On those systems, the SQLITE_THREAD_OVERRIDE_LOCK macro
** will be set appropriately.  On other systems, we have to check at
** runtime.  On these latter systems, SQLTIE_THREAD_OVERRIDE_LOCK is
** undefined.
**
** This variable normally has file scope only.  But during testing, we make
** it a global so that the test code can change its value in order to verify
** that the right stuff happens in either case.
*/
#ifndef SQLITE_THREAD_OVERRIDE_LOCK
# define SQLITE_THREAD_OVERRIDE_LOCK -1
#endif
#ifdef SQLITE_TEST
int threadsOverrideEachOthersLocks = SQLITE_THREAD_OVERRIDE_LOCK;
#else
static int threadsOverrideEachOthersLocks = SQLITE_THREAD_OVERRIDE_LOCK;
#endif

/*
** This structure holds information passed into individual test
** threads by the testThreadLockingBehavior() routine.
*/
struct threadTestData {
  int fd;                /* File to be locked */
  struct flock lock;     /* The locking operation */
  int result;            /* Result of the locking operation */
};

#ifdef SQLITE_LOCK_TRACE
/*
** Print out information about all locking operations.
**
** This routine is used for troubleshooting locks on multithreaded
** platforms.  Enable by compiling with the -DSQLITE_LOCK_TRACE
** command-line option on the compiler.  This code is normally
** turned off.
*/
static int lockTrace(int fd, int op, struct flock *p){
  char *zOpName, *zType;
  int s;
  int savedErrno;
  if( op==F_GETLK ){
    zOpName = "GETLK";
  }else if( op==F_SETLK ){
    zOpName = "SETLK";
  }else{
    s = fcntl(fd, op, p);
    sqlite3DebugPrintf("fcntl unknown %d %d %d\n", fd, op, s);
    return s;
  }
  if( p->l_type==F_RDLCK ){
    zType = "RDLCK";
  }else if( p->l_type==F_WRLCK ){
    zType = "WRLCK";
  }else if( p->l_type==F_UNLCK ){
    zType = "UNLCK";
  }else{
    assert( 0 );
  }
  assert( p->l_whence==SEEK_SET );
  s = fcntl(fd, op, p);
  savedErrno = errno;
  sqlite3DebugPrintf("fcntl %d %d %s %s %d %d %d %d\n",
     threadid, fd, zOpName, zType, (int)p->l_start, (int)p->l_len,
     (int)p->l_pid, s);
  if( s && op==F_SETLK && (p->l_type==F_RDLCK || p->l_type==F_WRLCK) ){
    struct flock l2;
    l2 = *p;
    fcntl(fd, F_GETLK, &l2);
    if( l2.l_type==F_RDLCK ){
      zType = "RDLCK";
    }else if( l2.l_type==F_WRLCK ){
      zType = "WRLCK";
    }else if( l2.l_type==F_UNLCK ){
      zType = "UNLCK";
    }else{
      assert( 0 );
    }
    sqlite3DebugPrintf("fcntl-failure-reason: %s %d %d %d\n",
       zType, (int)l2.l_start, (int)l2.l_len, (int)l2.l_pid);
  }
  errno = savedErrno;
  return s;
}
#define fcntl lockTrace
#endif /* SQLITE_LOCK_TRACE */

/*
** The testThreadLockingBehavior() routine launches two separate
** threads on this routine.  This routine attempts to lock a file
** descriptor then returns.  The success or failure of that attempt
** allows the testThreadLockingBehavior() procedure to determine
** whether or not threads can override each others locks.
*/
static void *threadLockingTest(void *pArg){
  struct threadTestData *pData = (struct threadTestData*)pArg;
  pData->result = fcntl(pData->fd, F_SETLK, &pData->lock);
  return pArg;
}

/*
** This procedure attempts to determine whether or not threads
** can override each others locks then sets the 
** threadsOverrideEachOthersLocks variable appropriately.
*/
static void testThreadLockingBehavior(int fd_orig){
  int fd;
  struct threadTestData d[2];
  pthread_t t[2];

  fd = dup(fd_orig);
  if( fd<0 ) return;
  memset(d, 0, sizeof(d));
  d[0].fd = fd;
  d[0].lock.l_type = F_RDLCK;
  d[0].lock.l_len = 1;
  d[0].lock.l_start = 0;
  d[0].lock.l_whence = SEEK_SET;
  d[1] = d[0];
  d[1].lock.l_type = F_WRLCK;
  pthread_create(&t[0], 0, threadLockingTest, &d[0]);
  pthread_create(&t[1], 0, threadLockingTest, &d[1]);
  pthread_join(t[0], 0);
  pthread_join(t[1], 0);
  close(fd);
  threadsOverrideEachOthersLocks =  d[0].result==0 && d[1].result==0;
}
#endif /* SQLITE_UNIX_THREADS */

/*
** Release a lockInfo structure previously allocated by findLockInfo().
*/
/* static void releaseLockInfo(struct lockInfo *pLock){ */
/*   assert( sqlite3OsInMutex(1) ); */
/*   pLock->nRef--; */
/*   if( pLock->nRef==0 ){ */
/*     sqlite3HashInsert(&lockHash, &pLock->key, sizeof(pLock->key), 0); */
/*     sqlite3ThreadSafeFree(pLock); */
/*   } */
/* } */

/*
** Release a openCnt structure previously allocated by findLockInfo().
*/
/* static void releaseOpenCnt(struct openCnt *pOpen){ */
/*   assert( sqlite3OsInMutex(1) ); */
/*   pOpen->nRef--; */
/*   if( pOpen->nRef==0 ){ */
/*     sqlite3HashInsert(&openHash, &pOpen->key, sizeof(pOpen->key), 0); */
/*     free(pOpen->aPending); */
/*     sqlite3ThreadSafeFree(pOpen); */
/*   } */
/* } */

/*
** Given a file descriptor, locate lockInfo and openCnt structures that
** describes that file descriptor.  Create new ones if necessary.  The
** return values might be uninitialized if an error occurs.
**
** Return the number of errors.
*/
/* static int findLockInfo( */
/*   int fd,                      /1* The file descriptor used in the key *1/ */
/*   struct lockInfo **ppLock,    /1* Return the lockInfo structure here *1/ */
/*   struct openCnt **ppOpen      /1* Return the openCnt structure here *1/ */
/* ){ */
/*   int rc; */
/*   struct lockKey key1; */
/*   struct openKey key2; */
/*   struct stat statbuf; */
/*   struct lockInfo *pLock; */
/*   struct openCnt *pOpen; */
/*   rc = fstat(fd, &statbuf); */
/*   if( rc!=0 ) return 1; */

/*   assert( sqlite3OsInMutex(1) ); */
/*   memset(&key1, 0, sizeof(key1)); */
/*   key1.dev = statbuf.st_dev; */
/*   key1.ino = statbuf.st_ino; */
/* #ifdef SQLITE_UNIX_THREADS */
/*   if( threadsOverrideEachOthersLocks<0 ){ */
/*     testThreadLockingBehavior(fd); */
/*   } */
/*   key1.tid = threadsOverrideEachOthersLocks ? 0 : pthread_self(); */
/* #endif */
/*   memset(&key2, 0, sizeof(key2)); */
/*   key2.dev = statbuf.st_dev; */
/*   key2.ino = statbuf.st_ino; */
/*   pLock = (struct lockInfo*)sqlite3HashFind(&lockHash, &key1, sizeof(key1)); */
/*   if( pLock==0 ){ */
/*     struct lockInfo *pOld; */
/*     pLock = sqlite3ThreadSafeMalloc( sizeof(*pLock) ); */
/*     if( pLock==0 ){ */
/*       rc = 1; */
/*       goto exit_findlockinfo; */
/*     } */
/*     pLock->key = key1; */
/*     pLock->nRef = 1; */
/*     pLock->cnt = 0; */
/*     pLock->locktype = 0; */
/*     pOld = sqlite3HashInsert(&lockHash, &pLock->key, sizeof(key1), pLock); */
/*     if( pOld!=0 ){ */
/*       assert( pOld==pLock ); */
/*       sqlite3ThreadSafeFree(pLock); */
/*       rc = 1; */
/*       goto exit_findlockinfo; */
/*     } */
/*   }else{ */
/*     pLock->nRef++; */
/*   } */
/*   *ppLock = pLock; */
/*   if( ppOpen!=0 ){ */
/*     pOpen = (struct openCnt*)sqlite3HashFind(&openHash, &key2, sizeof(key2)); */
/*     if( pOpen==0 ){ */
/*       struct openCnt *pOld; */
/*       pOpen = sqlite3ThreadSafeMalloc( sizeof(*pOpen) ); */
/*       if( pOpen==0 ){ */
/*         releaseLockInfo(pLock); */
/*         rc = 1; */
/*         goto exit_findlockinfo; */
/*       } */
/*       pOpen->key = key2; */
/*       pOpen->nRef = 1; */
/*       pOpen->nLock = 0; */
/*       pOpen->nPending = 0; */
/*       pOpen->aPending = 0; */
/*       pOld = sqlite3HashInsert(&openHash, &pOpen->key, sizeof(key2), pOpen); */
/*       if( pOld!=0 ){ */
/*         assert( pOld==pOpen ); */
/*         sqlite3ThreadSafeFree(pOpen); */
/*         releaseLockInfo(pLock); */
/*         rc = 1; */
/*         goto exit_findlockinfo; */
/*       } */
/*     }else{ */
/*       pOpen->nRef++; */
/*     } */
/*     *ppOpen = pOpen; */
/*   } */

/* exit_findlockinfo: */
/*   return rc; */
/* } */

#ifdef SQLITE_DEBUG
/*
** Helper function for printing out trace information from debugging
** binaries. This returns the string represetation of the supplied
** integer lock-type.
*/
static const char *locktypeName(int locktype){
  switch( locktype ){
  case NO_LOCK: return "NONE";
  case SHARED_LOCK: return "SHARED";
  case RESERVED_LOCK: return "RESERVED";
  case PENDING_LOCK: return "PENDING";
  case EXCLUSIVE_LOCK: return "EXCLUSIVE";
  }
  return "ERROR";
}
#endif

/*
** If we are currently in a different thread than the thread that the
** unixFile argument belongs to, then transfer ownership of the unixFile
** over to the current thread.
**
** A unixFile is only owned by a thread on systems where one thread is
** unable to override locks created by a different thread.  RedHat9 is
** an example of such a system.
**
** Ownership transfer is only allowed if the unixFile is currently unlocked.
** If the unixFile is locked and an ownership is wrong, then return
** SQLITE_MISUSE.  SQLITE_OK is returned if everything works.
*/
#ifdef SQLITE_UNIX_THREADS
static int transferOwnership(unixFile *pFile){
  int rc;
  pthread_t hSelf;
  if( threadsOverrideEachOthersLocks ){
    /* Ownership transfers not needed on this system */
    return SQLITE_OK;
  }
  hSelf = pthread_self();
  if( pthread_equal(pFile->tid, hSelf) ){
    /* We are still in the same thread */
    TRACE1("No-transfer, same thread\n");
    return SQLITE_OK;
  }
  if( pFile->locktype!=NO_LOCK ){
    /* We cannot change ownership while we are holding a lock! */
    return SQLITE_MISUSE;
  }
  TRACE4("Transfer ownership of %d from %d to %d\n", pFile->h,pFile->tid,hSelf);
  pFile->tid = hSelf;
  releaseLockInfo(pFile->pLock);
  rc = findLockInfo(pFile->h, &pFile->pLock, 0);
  TRACE5("LOCK    %d is now %s(%s,%d)\n", pFile->h,
     locktypeName(pFile->locktype),
     locktypeName(pFile->pLock->locktype), pFile->pLock->cnt);
  return rc;
}
#else
  /* On single-threaded builds, ownership transfer is a no-op */
# define transferOwnership(X) SQLITE_OK
#endif

/*
** Delete the named file
*/
int sqlite3UnixDelete(const char *zFilename){
  unlink(zFilename);
  return SQLITE_OK;
}

/*
** Return TRUE if the named file exists.
*/
int sqlite3UnixFileExists(const char *zFilename){
  return access(zFilename, 0)==0;
}

/* Forward declaration */
static int allocateUnixFile(unixFile *pInit, OsFile **pId);

/*
** Attempt to open a file for both reading and writing.  If that
** fails, try opening it read-only.  If the file does not exist,
** try to create it.
**
** On success, a handle for the open file is written to *id
** and *pReadonly is set to 0 if the file was opened for reading and
** writing or 1 if the file was opened read-only.  The function returns
** SQLITE_OK.
**
** On failure, the function returns SQLITE_CANTOPEN and leaves
** *id and *pReadonly unchanged.
*/
/* int sqlite3UnixOpenReadWrite( */
/*   const char *zFilename, */
/*   OsFile **pId, */
/*   int *pReadonly */
/* ){ */
/*   int rc; */
/*   unixFile f; */

/*   CRASH_TEST_OVERRIDE(sqlite3CrashOpenReadWrite, zFilename, pId, pReadonly); */
/*   assert( 0==*pId ); */
/*   f.h = open(zFilename, O_RDWR|O_CREAT|O_LARGEFILE|O_BINARY, */
/*                           SQLITE_DEFAULT_FILE_PERMISSIONS); */
/*   if( f.h<0 ){ */
/* #ifdef EISDIR */
/*     if( errno==EISDIR ){ */
/*       return SQLITE_CANTOPEN; */
/*     } */
/* #endif */
/*     f.h = open(zFilename, O_RDONLY|O_LARGEFILE|O_BINARY); */
/*     if( f.h<0 ){ */
/*       return SQLITE_CANTOPEN; */ 
/*     } */
/*     *pReadonly = 1; */
/*   }else{ */
/*     *pReadonly = 0; */
/*   } */
/*   sqlite3OsEnterMutex(); */
/*   rc = findLockInfo(f.h, &f.pLock, &f.pOpen); */
/*   sqlite3OsLeaveMutex(); */
/*   if( rc ){ */
/*     close(f.h); */
/*     return SQLITE_NOMEM; */
/*   } */
/*   TRACE3("OPEN    %-3d %s\n", f.h, zFilename); */
/*   return allocateUnixFile(&f, pId); */
/* } */


/*
** Attempt to open a new file for exclusive access by this process.
** The file will be opened for both reading and writing.  To avoid
** a potential security problem, we do not allow the file to have
** previously existed.  Nor do we allow the file to be a symbolic
** link.
**
** If delFlag is true, then make arrangements to automatically delete
** the file when it is closed.
**
** On success, write the file handle into *id and return SQLITE_OK.
**
** On failure, return SQLITE_CANTOPEN.
*/
/* int sqlite3UnixOpenExclusive(const char *zFilename, OsFile **pId, int delFlag){ */
/*   int rc; */
/*   unixFile f; */

/*   CRASH_TEST_OVERRIDE(sqlite3CrashOpenExclusive, zFilename, pId, delFlag); */
/*   assert( 0==*pId ); */
/*   if( access(zFilename, 0)==0 ){ */
/*     return SQLITE_CANTOPEN; */
/*   } */
/*   f.h = open(zFilename, */
/*                 O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW|O_LARGEFILE|O_BINARY, */
/*                 SQLITE_DEFAULT_FILE_PERMISSIONS); */
/*   if( f.h<0 ){ */
/*     return SQLITE_CANTOPEN; */
/*   } */
/*   sqlite3OsEnterMutex(); */
/*   rc = findLockInfo(f.h, &f.pLock, &f.pOpen); */
/*   sqlite3OsLeaveMutex(); */
/*   if( rc ){ */
/*     close(f.h); */
/*     unlink(zFilename); */
/*     return SQLITE_NOMEM; */
/*   } */
/*   if( delFlag ){ */
/*     unlink(zFilename); */
/*   } */
/*   TRACE3("OPEN-EX %-3d %s\n", f.h, zFilename); */
/*   return allocateUnixFile(&f, pId); */
/* } */

/*
** Attempt to open a new file for read-only access.
**
** On success, write the file handle into *id and return SQLITE_OK.
**
** On failure, return SQLITE_CANTOPEN.
*/
/* int sqlite3UnixOpenReadOnly(const char *zFilename, OsFile **pId){ */
/*   int rc; */
/*   unixFile f; */

/*   CRASH_TEST_OVERRIDE(sqlite3CrashOpenReadOnly, zFilename, pId, 0); */
/*   assert( 0==*pId ); */
/*   f.h = open(zFilename, O_RDONLY|O_LARGEFILE|O_BINARY); */
/*   if( f.h<0 ){ */
/*     return SQLITE_CANTOPEN; */
/*   } */
/*   sqlite3OsEnterMutex(); */
/*   rc = findLockInfo(f.h, &f.pLock, &f.pOpen); */
/*   sqlite3OsLeaveMutex(); */
/*   if( rc ){ */
/*     close(f.h); */
/*     return SQLITE_NOMEM; */
/*   } */
/*   TRACE3("OPEN-RO %-3d %s\n", f.h, zFilename); */
/*   return allocateUnixFile(&f, pId); */
/* } */

/*
** Attempt to open a file descriptor for the directory that contains a
** file.  This file descriptor can be used to fsync() the directory
** in order to make sure the creation of a new file is actually written
** to disk.
**
** This routine is only meaningful for Unix.  It is a no-op under
** windows since windows does not support hard links.
**
** On success, a handle for a previously open file at *id is
** updated with the new directory file descriptor and SQLITE_OK is
** returned.
**
** On failure, the function returns SQLITE_CANTOPEN and leaves
** *id unchanged.
*/
static int unixOpenDirectory(
  OsFile *id,
  const char *zDirname
){
  unixFile *pFile = (unixFile*)id;
  if( pFile==0 ){
    /* Do not open the directory if the corresponding file is not already
    ** open. */
    return SQLITE_CANTOPEN;
  }
  SET_THREADID(pFile);
  assert( pFile->dirfd<0 );
  pFile->dirfd = open(zDirname, O_RDONLY|O_BINARY, 0);
  if( pFile->dirfd<0 ){
    return SQLITE_CANTOPEN; 
  }
  TRACE3("OPENDIR %-3d %s\n", pFile->dirfd, zDirname);
  return SQLITE_OK;
}

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** temporary files.
**
** See also the "PRAGMA temp_store_directory" SQL command.
*/
char *sqlite3_temp_directory = 0;

/*
** Create a temporary file name in zBuf.  zBuf must be big enough to
** hold at least SQLITE_TEMPNAME_SIZE characters.
*/
/* int sqlite3UnixTempFileName(char *zBuf){ */
/*   static const char *azDirs[] = { */
/*      0, */
/*      "/var/tmp", */
/*      "/usr/tmp", */
/*      "/tmp", */
/*      ".", */
/*   }; */
/*   static const unsigned char zChars[] = */
/*     "abcdefghijklmnopqrstuvwxyz" */
/*     "ABCDEFGHIJKLMNOPQRSTUVWXYZ" */
/*     "0123456789"; */
/*   int i, j; */
/*   struct stat buf; */
/*   const char *zDir = "."; */
/*   azDirs[0] = sqlite3_temp_directory; */
/*   for(i=0; i<sizeof(azDirs)/sizeof(azDirs[0]); i++){ */
/*     if( azDirs[i]==0 ) continue; */
/*     if( stat(azDirs[i], &buf) ) continue; */
/*     if( !S_ISDIR(buf.st_mode) ) continue; */
/*     if( access(azDirs[i], 07) ) continue; */
/*     zDir = azDirs[i]; */
/*     break; */
/*   } */
/*   do{ */
/*     sprintf(zBuf, "%s/"TEMP_FILE_PREFIX, zDir); */
/*     j = strlen(zBuf); */
/*     sqlite3Randomness(15, &zBuf[j]); */
/*     for(i=0; i<15; i++, j++){ */
/*       zBuf[j] = (char)zChars[ ((unsigned char)zBuf[j])%(sizeof(zChars)-1) ]; */
/*     } */
/*     zBuf[j] = 0; */
/*   }while( access(zBuf,0)==0 ); */
/*   return SQLITE_OK; */ 
/* } */

/*
** Check that a given pathname is a directory and is writable 
**
*/
int sqlite3UnixIsDirWritable(char *zBuf){
#ifndef SQLITE_OMIT_PAGER_PRAGMAS
  struct stat buf;
  if( zBuf==0 ) return 0;
  if( zBuf[0]==0 ) return 0;
  if( stat(zBuf, &buf) ) return 0;
  if( !S_ISDIR(buf.st_mode) ) return 0;
  if( access(zBuf, 07) ) return 0;
#endif /* SQLITE_OMIT_PAGER_PRAGMAS */
  return 1;
}

/*
** Seek to the offset in id->offset then read cnt bytes into pBuf.
** Return the number of bytes actually read.  Update the offset.
*/
static int seekAndRead(unixFile *id, void *pBuf, int cnt){
  int got;
#ifdef USE_PREAD
  got = pread(id->h, pBuf, cnt, id->offset);
#else
  lseek(id->h, id->offset, SEEK_SET);
  got = read(id->h, pBuf, cnt);
#endif
  if( got>0 ){
    id->offset += got;
  }
  return got;
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
static int unixRead(OsFile *id, void *pBuf, int amt){
  int got;
  assert( id );
  SimulateIOError(SQLITE_IOERR);
  TIMER_START;
  got = seekAndRead((unixFile*)id, pBuf, amt);
  TIMER_END;
  TRACE5("READ    %-3d %5d %7d %d\n", ((unixFile*)id)->h, got,
          last_page, TIMER_ELAPSED);
  SEEK(0);
  /* if( got<0 ) got = 0; */
  if( got==amt ){
    return SQLITE_OK;
  }else{
    return SQLITE_IOERR;
  }
}

/*
** Seek to the offset in id->offset then read cnt bytes into pBuf.
** Return the number of bytes actually read.  Update the offset.
*/
static int seekAndWrite(unixFile *id, const void *pBuf, int cnt){
  int got;
#ifdef USE_PREAD
  got = pwrite(id->h, pBuf, cnt, id->offset);
#else
  lseek(id->h, id->offset, SEEK_SET);
  got = write(id->h, pBuf, cnt);
#endif
  if( got>0 ){
    id->offset += got;
  }
  return got;
}


/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
static int unixWrite(OsFile *id, const void *pBuf, int amt){
  int wrote = 0;
  assert( id );
  assert( amt>0 );
  SimulateIOError(SQLITE_IOERR);
  SimulateDiskfullError;
  TIMER_START;
  while( amt>0 && (wrote = seekAndWrite((unixFile*)id, pBuf, amt))>0 ){
    amt -= wrote;
    pBuf = &((char*)pBuf)[wrote];
  }
  TIMER_END;
  TRACE5("WRITE   %-3d %5d %7d %d\n", ((unixFile*)id)->h, wrote,
          last_page, TIMER_ELAPSED);
  SEEK(0);
  if( amt>0 ){
    return SQLITE_FULL;
  }
  return SQLITE_OK;
}

/*
** Move the read/write pointer in a file.
*/
static int unixSeek(OsFile *id, i64 offset){
  assert( id );
  SEEK(offset/1024 + 1);
#ifdef SQLITE_TEST
  if( offset ) SimulateDiskfullError
#endif
  ((unixFile*)id)->offset = offset;
  return SQLITE_OK;
}

#ifdef SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occuring at the right times.
*/
int sqlite3_sync_count = 0;
int sqlite3_fullsync_count = 0;
#endif

/*
** Use the fdatasync() API only if the HAVE_FDATASYNC macro is defined.
** Otherwise use fsync() in its place.
*/
#ifndef HAVE_FDATASYNC
# define fdatasync fsync
#endif

/*
** Define HAVE_FULLFSYNC to 0 or 1 depending on whether or not
** the F_FULLFSYNC macro is defined.  F_FULLFSYNC is currently
** only available on Mac OS X.  But that could change.
*/
#ifdef F_FULLFSYNC
# define HAVE_FULLFSYNC 1
#else
# define HAVE_FULLFSYNC 0
#endif


/*
** The fsync() system call does not work as advertised on many
** unix systems.  The following procedure is an attempt to make
** it work better.
**
** The SQLITE_NO_SYNC macro disables all fsync()s.  This is useful
** for testing when we want to run through the test suite quickly.
** You are strongly advised *not* to deploy with SQLITE_NO_SYNC
** enabled, however, since with SQLITE_NO_SYNC enabled, an OS crash
** or power failure will likely corrupt the database file.
*/
static int full_fsync(int fd, int fullSync, int dataOnly){
  int rc;

  /* Record the number of times that we do a normal fsync() and 
  ** FULLSYNC.  This is used during testing to verify that this procedure
  ** gets called with the correct arguments.
  */
#ifdef SQLITE_TEST
  if( fullSync ) sqlite3_fullsync_count++;
  sqlite3_sync_count++;
#endif

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#ifdef SQLITE_NO_SYNC
  rc = SQLITE_OK;
#else

#if HAVE_FULLFSYNC
  if( fullSync ){
    rc = fcntl(fd, F_FULLFSYNC, 0);
  }else{
    rc = 1;
  }
  /* If the FULLSYNC failed, try to do a normal fsync() */
  if( rc ) rc = fsync(fd);

#else /* if !defined(F_FULLSYNC) */
  if( dataOnly ){
    rc = fdatasync(fd);
  }else{
    rc = fsync(fd);
  }
#endif /* defined(F_FULLFSYNC) */
#endif /* defined(SQLITE_NO_SYNC) */

  return rc;
}

/*
** Make sure all writes to a particular file are committed to disk.
**
** If dataOnly==0 then both the file itself and its metadata (file
** size, access time, etc) are synced.  If dataOnly!=0 then only the
** file data is synced.
**
** Under Unix, also make sure that the directory entry for the file
** has been created by fsync-ing the directory that contains the file.
** If we do not do this and we encounter a power failure, the directory
** entry for the journal might not exist after we reboot.  The next
** SQLite to access the file will not know that the journal exists (because
** the directory entry for the journal was never created) and the transaction
** will not roll back - possibly leading to database corruption.
*/
static int unixSync(OsFile *id, int dataOnly){
  unixFile *pFile = (unixFile*)id;
  assert( pFile );
  SimulateIOError(SQLITE_IOERR);
  TRACE2("SYNC    %-3d\n", pFile->h);
  if( full_fsync(pFile->h, pFile->fullSync, dataOnly) ){
    return SQLITE_IOERR;
  }
  if( pFile->dirfd>=0 ){
    TRACE4("DIRSYNC %-3d (have_fullfsync=%d fullsync=%d)\n", pFile->dirfd,
            HAVE_FULLFSYNC, pFile->fullSync);
#ifndef SQLITE_DISABLE_DIRSYNC
    /* The directory sync is only attempted if full_fsync is
    ** turned off or unavailable.  If a full_fsync occurred above,
    ** then the directory sync is superfluous.
    */
    if( (!HAVE_FULLFSYNC || !pFile->fullSync) && full_fsync(pFile->dirfd,0,0) ){
       /*
       ** We have received multiple reports of fsync() returning
       ** errors when applied to directories on certain file systems.
       ** A failed directory sync is not a big deal.  So it seems
       ** better to ignore the error.  Ticket #1657
       */
       /* return SQLITE_IOERR; */
    }
#endif
    close(pFile->dirfd);  /* Only need to sync once, so close the directory */
    pFile->dirfd = -1;    /* when we are done. */
  }
  return SQLITE_OK;
}

/*
** Sync the directory zDirname. This is a no-op on operating systems other
** than UNIX.
**
** This is used to make sure the master journal file has truely been deleted
** before making changes to individual journals on a multi-database commit.
** The F_FULLFSYNC option is not needed here.
*/
int sqlite3UnixSyncDirectory(const char *zDirname){
#ifdef SQLITE_DISABLE_DIRSYNC
  return SQLITE_OK;
#else
  int fd;
  int r;
  SimulateIOError(SQLITE_IOERR);
  fd = open(zDirname, O_RDONLY|O_BINARY, 0);
  TRACE3("DIRSYNC %-3d (%s)\n", fd, zDirname);
  if( fd<0 ){
    return SQLITE_CANTOPEN; 
  }
  r = fsync(fd);
  close(fd);
  return ((r==0)?SQLITE_OK:SQLITE_IOERR);
#endif
}

/*
** Truncate an open file to a specified size
*/
static int unixTruncate(OsFile *id, i64 nByte){
  assert( id );
  SimulateIOError(SQLITE_IOERR);
  return ftruncate(((unixFile*)id)->h, nByte)==0 ? SQLITE_OK : SQLITE_IOERR;
}

/*
** Determine the current size of a file in bytes
*/
static int unixFileSize(OsFile *id, i64 *pSize){
  struct stat buf;
  assert( id );
  SimulateIOError(SQLITE_IOERR);
  if( fstat(((unixFile*)id)->h, &buf)!=0 ){
    return SQLITE_IOERR;
  }
  *pSize = buf.st_size;
  return SQLITE_OK;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero.  If the file is unlocked or holds only SHARED locks, then
** return zero.
*/
static int unixCheckReservedLock(OsFile *id){
  int r = 0;
  unixFile *pFile = (unixFile*)id;

  assert( pFile );
  sqlite3OsEnterMutex(); /* Because pFile->pLock is shared across threads */

  /* Check if a thread in this process holds such a lock */
  if( pFile->pLock->locktype>SHARED_LOCK ){
    r = 1;
  }

  /* Otherwise see if some other process holds it.
  */
  if( !r ){
    struct flock lock;
    lock.l_whence = SEEK_SET;
    lock.l_start = RESERVED_BYTE;
    lock.l_len = 1;
    lock.l_type = F_WRLCK;
    fcntl(pFile->h, F_GETLK, &lock);
    if( lock.l_type!=F_UNLCK ){
      r = 1;
    }
  }
  
  sqlite3OsLeaveMutex();
  TRACE3("TEST WR-LOCK %d %d\n", pFile->h, r);

  return r;
}

/*
** Lock the file with the lock specified by parameter locktype - one
** of the following:
**
**     (1) SHARED_LOCK
**     (2) RESERVED_LOCK
**     (3) PENDING_LOCK
**     (4) EXCLUSIVE_LOCK
**
** Sometimes when requesting one lock state, additional lock states
** are inserted in between.  The locking might fail on one of the later
** transitions leaving the lock state different from what it started but
** still short of its goal.  The following chart shows the allowed
** transitions and the inserted intermediate states:
**
**    UNLOCKED -> SHARED
**    SHARED -> RESERVED
**    SHARED -> (PENDING) -> EXCLUSIVE
**    RESERVED -> (PENDING) -> EXCLUSIVE
**    PENDING -> EXCLUSIVE
**
** This routine will only increase a lock.  Use the sqlite3OsUnlock()
** routine to lower a locking level.
*/
static int unixLock(OsFile *id, int locktype){
  /* The following describes the implementation of the various locks and
  ** lock transitions in terms of the POSIX advisory shared and exclusive
  ** lock primitives (called read-locks and write-locks below, to avoid
  ** confusion with SQLite lock names). The algorithms are complicated
  ** slightly in order to be compatible with windows systems simultaneously
  ** accessing the same database file, in case that is ever required.
  **
  ** Symbols defined in os.h indentify the 'pending byte' and the 'reserved
  ** byte', each single bytes at well known offsets, and the 'shared byte
  ** range', a range of 510 bytes at a well known offset.
  **
  ** To obtain a SHARED lock, a read-lock is obtained on the 'pending
  ** byte'.  If this is successful, a random byte from the 'shared byte
  ** range' is read-locked and the lock on the 'pending byte' released.
  **
  ** A process may only obtain a RESERVED lock after it has a SHARED lock.
  ** A RESERVED lock is implemented by grabbing a write-lock on the
  ** 'reserved byte'. 
  **
  ** A process may only obtain a PENDING lock after it has obtained a
  ** SHARED lock. A PENDING lock is implemented by obtaining a write-lock
  ** on the 'pending byte'. This ensures that no new SHARED locks can be
  ** obtained, but existing SHARED locks are allowed to persist. A process
  ** does not have to obtain a RESERVED lock on the way to a PENDING lock.
  ** This property is used by the algorithm for rolling back a journal file
  ** after a crash.
  **
  ** An EXCLUSIVE lock, obtained after a PENDING lock is held, is
  ** implemented by obtaining a write-lock on the entire 'shared byte
  ** range'. Since all other locks require a read-lock on one of the bytes
  ** within this range, this ensures that no other locks are held on the
  ** database. 
  **
  ** The reason a single byte cannot be used instead of the 'shared byte
  ** range' is that some versions of windows do not support read-locks. By
  ** locking a random byte from a range, concurrent SHARED locks may exist
  ** even if the locking primitive used is always a write-lock.
  */
  int rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  struct lockInfo *pLock = pFile->pLock;
  struct flock lock;
  int s;

  assert( pFile );
  TRACE7("LOCK    %d %s was %s(%s,%d) pid=%d\n", pFile->h,
      locktypeName(locktype), locktypeName(pFile->locktype),
      locktypeName(pLock->locktype), pLock->cnt , getpid());

  /* If there is already a lock of this type or more restrictive on the
  ** OsFile, do nothing. Don't use the end_lock: exit path, as
  ** sqlite3OsEnterMutex() hasn't been called yet.
  */
  if( pFile->locktype>=locktype ){
    TRACE3("LOCK    %d %s ok (already held)\n", pFile->h,
            locktypeName(locktype));
    return SQLITE_OK;
  }

  /* Make sure the locking sequence is correct
  */
  assert( pFile->locktype!=NO_LOCK || locktype==SHARED_LOCK );
  assert( locktype!=PENDING_LOCK );
  assert( locktype!=RESERVED_LOCK || pFile->locktype==SHARED_LOCK );

  /* This mutex is needed because pFile->pLock is shared across threads
  */
  sqlite3OsEnterMutex();

  /* Make sure the current thread owns the pFile.
  */
  rc = transferOwnership(pFile);
  if( rc!=SQLITE_OK ){
    sqlite3OsLeaveMutex();
    return rc;
  }
  pLock = pFile->pLock;

  /* If some thread using this PID has a lock via a different OsFile*
  ** handle that precludes the requested lock, return BUSY.
  */
  if( (pFile->locktype!=pLock->locktype && 
          (pLock->locktype>=PENDING_LOCK || locktype>SHARED_LOCK))
  ){
    rc = SQLITE_BUSY;
    goto end_lock;
  }

  /* If a SHARED lock is requested, and some thread using this PID already
  ** has a SHARED or RESERVED lock, then increment reference counts and
  ** return SQLITE_OK.
  */
  if( locktype==SHARED_LOCK && 
      (pLock->locktype==SHARED_LOCK || pLock->locktype==RESERVED_LOCK) ){
    assert( locktype==SHARED_LOCK );
    assert( pFile->locktype==0 );
    assert( pLock->cnt>0 );
    pFile->locktype = SHARED_LOCK;
    pLock->cnt++;
    pFile->pOpen->nLock++;
    goto end_lock;
  }

  lock.l_len = 1L;

  lock.l_whence = SEEK_SET;

  /* A PENDING lock is needed before acquiring a SHARED lock and before
  ** acquiring an EXCLUSIVE lock.  For the SHARED lock, the PENDING will
  ** be released.
  */
  if( locktype==SHARED_LOCK 
      || (locktype==EXCLUSIVE_LOCK && pFile->locktype<PENDING_LOCK)
  ){
    lock.l_type = (locktype==SHARED_LOCK?F_RDLCK:F_WRLCK);
    lock.l_start = PENDING_BYTE;
    s = fcntl(pFile->h, F_SETLK, &lock);
    if( s ){
      rc = (errno==EINVAL) ? SQLITE_NOLFS : SQLITE_BUSY;
      goto end_lock;
    }
  }


  /* If control gets to this point, then actually go ahead and make
  ** operating system calls for the specified lock.
  */
  if( locktype==SHARED_LOCK ){
    assert( pLock->cnt==0 );
    assert( pLock->locktype==0 );

    /* Now get the read-lock */
    lock.l_start = SHARED_FIRST;
    lock.l_len = SHARED_SIZE;
    s = fcntl(pFile->h, F_SETLK, &lock);

    /* Drop the temporary PENDING lock */
    lock.l_start = PENDING_BYTE;
    lock.l_len = 1L;
    lock.l_type = F_UNLCK;
    if( fcntl(pFile->h, F_SETLK, &lock)!=0 ){
      rc = SQLITE_IOERR;  /* This should never happen */
      goto end_lock;
    }
    if( s ){
      rc = (errno==EINVAL) ? SQLITE_NOLFS : SQLITE_BUSY;
    }else{
      pFile->locktype = SHARED_LOCK;
      pFile->pOpen->nLock++;
      pLock->cnt = 1;
    }
  }else if( locktype==EXCLUSIVE_LOCK && pLock->cnt>1 ){
    /* We are trying for an exclusive lock but another thread in this
    ** same process is still holding a shared lock. */
    rc = SQLITE_BUSY;
  }else{
    /* The request was for a RESERVED or EXCLUSIVE lock.  It is
    ** assumed that there is a SHARED or greater lock on the file
    ** already.
    */
    assert( 0!=pFile->locktype );
    lock.l_type = F_WRLCK;
    switch( locktype ){
      case RESERVED_LOCK:
        lock.l_start = RESERVED_BYTE;
        break;
      case EXCLUSIVE_LOCK:
        lock.l_start = SHARED_FIRST;
        lock.l_len = SHARED_SIZE;
        break;
      default:
        assert(0);
    }
    s = fcntl(pFile->h, F_SETLK, &lock);
    if( s ){
      rc = (errno==EINVAL) ? SQLITE_NOLFS : SQLITE_BUSY;
    }
  }
  
  if( rc==SQLITE_OK ){
    pFile->locktype = locktype;
    pLock->locktype = locktype;
  }else if( locktype==EXCLUSIVE_LOCK ){
    pFile->locktype = PENDING_LOCK;
    pLock->locktype = PENDING_LOCK;
  }

end_lock:
  sqlite3OsLeaveMutex();
  TRACE4("LOCK    %d %s %s\n", pFile->h, locktypeName(locktype), 
      rc==SQLITE_OK ? "ok" : "failed");
  return rc;
}

/*
** Lower the locking level on file descriptor pFile to locktype.  locktype
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
static int unixUnlock(OsFile *id, int locktype){
  struct lockInfo *pLock;
  struct flock lock;
  int rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;

  assert( pFile );
  TRACE7("UNLOCK  %d %d was %d(%d,%d) pid=%d\n", pFile->h, locktype,
      pFile->locktype, pFile->pLock->locktype, pFile->pLock->cnt, getpid());

  assert( locktype<=SHARED_LOCK );
  if( pFile->locktype<=locktype ){
    return SQLITE_OK;
  }
  if( CHECK_THREADID(pFile) ){
    return SQLITE_MISUSE;
  }
  sqlite3OsEnterMutex();
  pLock = pFile->pLock;
  assert( pLock->cnt!=0 );
  if( pFile->locktype>SHARED_LOCK ){
    assert( pLock->locktype==pFile->locktype );
    if( locktype==SHARED_LOCK ){
      lock.l_type = F_RDLCK;
      lock.l_whence = SEEK_SET;
      lock.l_start = SHARED_FIRST;
      lock.l_len = SHARED_SIZE;
      if( fcntl(pFile->h, F_SETLK, &lock)!=0 ){
        /* This should never happen */
        rc = SQLITE_IOERR;
      }
    }
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = PENDING_BYTE;
    lock.l_len = 2L;  assert( PENDING_BYTE+1==RESERVED_BYTE );
    if( fcntl(pFile->h, F_SETLK, &lock)==0 ){
      pLock->locktype = SHARED_LOCK;
    }else{
      rc = SQLITE_IOERR;  /* This should never happen */
    }
  }
  if( locktype==NO_LOCK ){
    struct openCnt *pOpen;

    /* Decrement the shared lock counter.  Release the lock using an
    ** OS call only when all threads in this same process have released
    ** the lock.
    */
    pLock->cnt--;
    if( pLock->cnt==0 ){
      lock.l_type = F_UNLCK;
      lock.l_whence = SEEK_SET;
      lock.l_start = lock.l_len = 0L;
      if( fcntl(pFile->h, F_SETLK, &lock)==0 ){
        pLock->locktype = NO_LOCK;
      }else{
        rc = SQLITE_IOERR;  /* This should never happen */
      }
    }

    /* Decrement the count of locks against this same file.  When the
    ** count reaches zero, close any other file descriptors whose close
    ** was deferred because of outstanding locks.
    */
    pOpen = pFile->pOpen;
    pOpen->nLock--;
    assert( pOpen->nLock>=0 );
    if( pOpen->nLock==0 && pOpen->nPending>0 ){
      int i;
      for(i=0; i<pOpen->nPending; i++){
        close(pOpen->aPending[i]);
      }
      free(pOpen->aPending);
      pOpen->nPending = 0;
      pOpen->aPending = 0;
    }
  }
  sqlite3OsLeaveMutex();
  pFile->locktype = locktype;
  return rc;
}

/*
** Close a file.
*/
/* static int unixClose(OsFile **pId){ */
/*   unixFile *id = (unixFile*)*pId; */

/*   if( !id ) return SQLITE_OK; */
/*   unixUnlock(*pId, NO_LOCK); */
/*   if( id->dirfd>=0 ) close(id->dirfd); */
/*   id->dirfd = -1; */
/*   sqlite3OsEnterMutex(); */

/*   if( id->pOpen->nLock ){ */
/*     /1* If there are outstanding locks, do not actually close the file just */
/*     ** yet because that would clear those locks.  Instead, add the file */
/*     ** descriptor to pOpen->aPending.  It will be automatically closed when */
/*     ** the last lock is cleared. */
/*     *1/ */
/*     int *aNew; */
/*     struct openCnt *pOpen = id->pOpen; */
/*     aNew = realloc( pOpen->aPending, (pOpen->nPending+1)*sizeof(int) ); */
/*     if( aNew==0 ){ */
/*       /1* If a malloc fails, just leak the file descriptor *1/ */
/*     }else{ */
/*       pOpen->aPending = aNew; */
/*       pOpen->aPending[pOpen->nPending] = id->h; */
/*       pOpen->nPending++; */
/*     } */
/*   }else{ */
/*     /1* There are no outstanding locks so we can close the file immediately *1/ */
/*     close(id->h); */
/*   } */
/*   releaseLockInfo(id->pLock); */
/*   releaseOpenCnt(id->pOpen); */

/*   sqlite3OsLeaveMutex(); */
/*   id->isOpen = 0; */
/*   TRACE2("CLOSE   %-3d\n", id->h); */
/*   OpenCounter(-1); */
/*   sqlite3ThreadSafeFree(id); */
/*   *pId = 0; */
/*   return SQLITE_OK; */
/* } */

/*
** Turn a relative pathname into a full pathname.  Return a pointer
** to the full pathname stored in space obtained from sqliteMalloc().
** The calling function is responsible for freeing this space once it
** is no longer needed.
*/
char *sqlite3UnixFullPathname(const char *zRelative){
  char *zFull = 0;
  if( zRelative[0]=='/' ){
    sqlite3SetString(&zFull, zRelative, (char*)0);
  }else{
    char *zBuf = sqliteMalloc(5000);
    if( zBuf==0 ){
      return 0;
    }
    zBuf[0] = 0;
    sqlite3SetString(&zFull, getcwd(zBuf, 5000), "/", zRelative,
                    (char*)0);
    sqliteFree(zBuf);
  }

#if 0
  /*
  ** Remove "/./" path elements and convert "/A/./" path elements
  ** to just "/".
  */
  if( zFull ){
    int i, j;
    for(i=j=0; zFull[i]; i++){
      if( zFull[i]=='/' ){
        if( zFull[i+1]=='/' ) continue;
        if( zFull[i+1]=='.' && zFull[i+2]=='/' ){
          i += 1;
          continue;
        }
        if( zFull[i+1]=='.' && zFull[i+2]=='.' && zFull[i+3]=='/' ){
          while( j>0 && zFull[j-1]!='/' ){ j--; }
          i += 3;
          continue;
        }
      }
      zFull[j++] = zFull[i];
    }
    zFull[j] = 0;
  }
#endif

  return zFull;
}

/*
** Change the value of the fullsync flag in the given file descriptor.
*/
static void unixSetFullSync(OsFile *id, int v){
  ((unixFile*)id)->fullSync = v;
}

/*
** Return the underlying file handle for an OsFile
*/
static int unixFileHandle(OsFile *id){
  return ((unixFile*)id)->h;
}

/*
** Return an integer that indices the type of lock currently held
** by this handle.  (Used for testing and analysis only.)
*/
static int unixLockState(OsFile *id){
  return ((unixFile*)id)->locktype;
}

/*
** This vector defines all the methods that can operate on an OsFile
** for unix.
*/
// static const IoMethod sqlite3UnixIoMethod = {
//   //unixClose,
//   unixOpenDirectory,
//   unixRead,
//   unixWrite,
//   unixSeek,
//   unixTruncate,
//   unixSync,
//   unixSetFullSync,
//   unixFileHandle,
//   unixFileSize,
//   unixLock,
//   unixUnlock,
//   unixLockState,
//   unixCheckReservedLock,
// };

/*
** Allocate memory for a unixFile.  Initialize the new unixFile
** to the value given in pInit and return a pointer to the new
** OsFile.  If we run out of memory, close the file and return NULL.
*/
/* static int allocateUnixFile(unixFile *pInit, OsFile **pId){ */
/*   unixFile *pNew; */
/*   pInit->dirfd = -1; */
/*   pInit->fullSync = 0; */
/*   pInit->locktype = 0; */
/*   pInit->offset = 0; */
/*   SET_THREADID(pInit); */
/*   pNew = sqlite3ThreadSafeMalloc( sizeof(unixFile) ); */
/*   if( pNew==0 ){ */
/*     close(pInit->h); */
/*     sqlite3OsEnterMutex(); */
/*     releaseLockInfo(pInit->pLock); */
/*     releaseOpenCnt(pInit->pOpen); */
/*     sqlite3OsLeaveMutex(); */
/*     *pId = 0; */
/*     return SQLITE_NOMEM; */
/*   }else{ */
/*     *pNew = *pInit; */
/*     pNew->pMethod = &sqlite3UnixIoMethod; */
/*     *pId = (OsFile*)pNew; */
/*     OpenCounter(+1); */
/*     return SQLITE_OK; */
/*   } */
/* } */


#endif /* SQLITE_OMIT_DISKIO */
/***************************************************************************
** Everything above deals with file I/O.  Everything that follows deals
** with other miscellanous aspects of the operating system interface
****************************************************************************/


/*
** Get information to seed the random number generator.  The seed
** is written into the buffer zBuf[256].  The calling function must
** supply a sufficiently large buffer.
*/
int sqlite3UnixRandomSeed(char *zBuf){
  /* We have to initialize zBuf to prevent valgrind from reporting
  ** errors.  The reports issued by valgrind are incorrect - we would
  ** prefer that the randomness be increased by making use of the
  ** uninitialized space in zBuf - but valgrind errors tend to worry
  ** some users.  Rather than argue, it seems easier just to initialize
  ** the whole array and silence valgrind, even if that means less randomness
  ** in the random seed.
  **
  ** When testing, initializing zBuf[] to zero is all we do.  That means
  ** that we always use the same random number sequence.  This makes the
  ** tests repeatable.
  */
  memset(zBuf, 0, 256);
#if !defined(SQLITE_TEST)
  {
    int pid, fd;
    fd = open("/dev/urandom", O_RDONLY);
    if( fd<0 ){
      time_t t;
      time(&t);
      memcpy(zBuf, &t, sizeof(t));
      pid = getpid();
      memcpy(&zBuf[sizeof(time_t)], &pid, sizeof(pid));
    }else{
      read(fd, zBuf, 256);
      close(fd);
    }
  }
#endif
  return SQLITE_OK;
}

/*
** Sleep for a little while.  Return the amount of time slept.
** The argument is the number of milliseconds we want to sleep.
*/
int sqlite3UnixSleep(int ms){
#if defined(HAVE_USLEEP) && HAVE_USLEEP
  usleep(ms*1000);
  return ms;
#else
  sleep((ms+999)/1000);
  return 1000*((ms+999)/1000);
#endif
}

/*
** Static variables used for thread synchronization.
**
** inMutex      the nesting depth of the recursive mutex.  The thread
**              holding mutexMain can read this variable at any time.
**              But is must hold mutexAux to change this variable.  Other
**              threads must hold mutexAux to read the variable and can
**              never write.
**
** mutexOwner   The thread id of the thread holding mutexMain.  Same
**              access rules as for inMutex.
**
** mutexOwnerValid   True if the value in mutexOwner is valid.  The same
**                   access rules apply as for inMutex.
**
** mutexMain    The main mutex.  Hold this mutex in order to get exclusive
**              access to SQLite data structures.
**
** mutexAux     An auxiliary mutex needed to access variables defined above.
**
** Mutexes are always acquired in this order: mutexMain mutexAux.   It
** is not necessary to acquire mutexMain in order to get mutexAux - just
** do not attempt to acquire them in the reverse order: mutexAux mutexMain.
** Either get the mutexes with mutexMain first or get mutexAux only.
**
** When running on a platform where the three variables inMutex, mutexOwner,
** and mutexOwnerValid can be set atomically, the mutexAux is not required.
** On many systems, all three are 32-bit integers and writing to a 32-bit
** integer is atomic.  I think.  But there are no guarantees.  So it seems
** safer to protect them using mutexAux.
*/
static int inMutex = 0;
#ifdef SQLITE_UNIX_THREADS
static pthread_t mutexOwner;          /* Thread holding mutexMain */
static int mutexOwnerValid = 0;       /* True if mutexOwner is valid */
static pthread_mutex_t mutexMain = PTHREAD_MUTEX_INITIALIZER; /* The mutex */
static pthread_mutex_t mutexAux = PTHREAD_MUTEX_INITIALIZER;  /* Aux mutex */
#endif

/*
** The following pair of routine implement mutual exclusion for
** multi-threaded processes.  Only a single thread is allowed to
** executed code that is surrounded by EnterMutex() and LeaveMutex().
**
** SQLite uses only a single Mutex.  There is not much critical
** code and what little there is executes quickly and without blocking.
**
** As of version 3.3.2, this mutex must be recursive.
*/
void sqlite3UnixEnterMutex(){
#ifdef SQLITE_UNIX_THREADS
  pthread_mutex_lock(&mutexAux);
  if( !mutexOwnerValid || !pthread_equal(mutexOwner, pthread_self()) ){
    pthread_mutex_unlock(&mutexAux);
    pthread_mutex_lock(&mutexMain);
    assert( inMutex==0 );
    assert( !mutexOwnerValid );
    pthread_mutex_lock(&mutexAux);
    mutexOwner = pthread_self();
    mutexOwnerValid = 1;
  }
  inMutex++;
  pthread_mutex_unlock(&mutexAux);
#else
  inMutex++;
#endif
}
void sqlite3UnixLeaveMutex(){
  assert( inMutex>0 );
#ifdef SQLITE_UNIX_THREADS
  pthread_mutex_lock(&mutexAux);
  inMutex--;
  assert( pthread_equal(mutexOwner, pthread_self()) );
  if( inMutex==0 ){
    assert( mutexOwnerValid );
    mutexOwnerValid = 0;
    pthread_mutex_unlock(&mutexMain);
  }
  pthread_mutex_unlock(&mutexAux);
#else
  inMutex--;
#endif
}

/*
** Return TRUE if the mutex is currently held.
**
** If the thisThrd parameter is true, return true only if the
** calling thread holds the mutex.  If the parameter is false, return
** true if any thread holds the mutex.
*/
int sqlite3UnixInMutex(int thisThrd){
#ifdef SQLITE_UNIX_THREADS
  int rc;
  pthread_mutex_lock(&mutexAux);
  rc = inMutex>0 && (thisThrd==0 || pthread_equal(mutexOwner,pthread_self()));
  pthread_mutex_unlock(&mutexAux);
  return rc;
#else
  return inMutex>0;
#endif
}

/*
** Remember the number of thread-specific-data blocks allocated.
** Use this to verify that we are not leaking thread-specific-data.
** Ticket #1601
*/
#ifdef SQLITE_TEST
int sqlite3_tsd_count = 0;
# ifdef SQLITE_UNIX_THREADS
    static pthread_mutex_t tsd_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
#   define TSD_COUNTER(N) \
             pthread_mutex_lock(&tsd_counter_mutex); \
             sqlite3_tsd_count += N; \
             pthread_mutex_unlock(&tsd_counter_mutex);
# else
#   define TSD_COUNTER(N)  sqlite3_tsd_count += N
# endif
#else
# define TSD_COUNTER(N)  /* no-op */
#endif

/*
** If called with allocateFlag>0, then return a pointer to thread
** specific data for the current thread.  Allocate and zero the
** thread-specific data if it does not already exist.
**
** If called with allocateFlag==0, then check the current thread
** specific data.  Return it if it exists.  If it does not exist,
** then return NULL.
**
** If called with allocateFlag<0, check to see if the thread specific
** data is allocated and is all zero.  If it is then deallocate it.
** Return a pointer to the thread specific data or NULL if it is
** unallocated or gets deallocated.
*/
ThreadData *sqlite3UnixThreadSpecificData(int allocateFlag){
  static const ThreadData zeroData = {0};  /* Initializer to silence warnings
                                           ** from broken compilers */
#ifdef SQLITE_UNIX_THREADS
  static pthread_key_t key;
  static int keyInit = 0;
  ThreadData *pTsd;

  if( !keyInit ){
    sqlite3OsEnterMutex();
    if( !keyInit ){
      int rc;
      rc = pthread_key_create(&key, 0);
      if( rc ){
        sqlite3OsLeaveMutex();
        return 0;
      }
      keyInit = 1;
    }
    sqlite3OsLeaveMutex();
  }

  pTsd = pthread_getspecific(key);
  if( allocateFlag>0 ){
    if( pTsd==0 ){
      if( !sqlite3TestMallocFail() ){
        pTsd = sqlite3OsMalloc(sizeof(zeroData));
      }
#ifdef SQLITE_MEMDEBUG
      sqlite3_isFail = 0;
#endif
      if( pTsd ){
        *pTsd = zeroData;
        pthread_setspecific(key, pTsd);
        TSD_COUNTER(+1);
      }
    }
  }else if( pTsd!=0 && allocateFlag<0 
            && memcmp(pTsd, &zeroData, sizeof(ThreadData))==0 ){
    sqlite3OsFree(pTsd);
    pthread_setspecific(key, 0);
    TSD_COUNTER(-1);
    pTsd = 0;
  }
  return pTsd;
#else
  static ThreadData *pTsd = 0;
  if( allocateFlag>0 ){
    if( pTsd==0 ){
      if( !sqlite3TestMallocFail() ){
        pTsd = sqlite3OsMalloc( sizeof(zeroData) );
      }
#ifdef SQLITE_MEMDEBUG
      sqlite3_isFail = 0;
#endif
      if( pTsd ){
        *pTsd = zeroData;
        TSD_COUNTER(+1);
      }
    }
  }else if( pTsd!=0 && allocateFlag<0
            && memcmp(pTsd, &zeroData, sizeof(ThreadData))==0 ){
    sqlite3OsFree(pTsd);
    TSD_COUNTER(-1);
    pTsd = 0;
  }
  return pTsd;
#endif
}

/*
** The following variable, if set to a non-zero value, becomes the result
** returned from sqlite3OsCurrentTime().  This is used for testing.
*/
#ifdef SQLITE_TEST
int sqlite3_current_time = 0;
#endif

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
int sqlite3UnixCurrentTime(double *prNow){
#ifdef NO_GETTOD
  time_t t;
  time(&t);
  *prNow = t/86400.0 + 2440587.5;
#else
  struct timeval sNow;
  //struct timezone sTz;  /* Not used */
  gettimeofday(&sNow, 0);
  *prNow = 2440587.5 + sNow.tv_sec/86400.0 + sNow.tv_usec/86400000000.0;
#endif
#ifdef SQLITE_TEST
  if( sqlite3_current_time ){
    *prNow = sqlite3_current_time/86400.0 + 2440587.5;
  }
#endif
  return 0;
}

#endif /* OS_UNIX */

/************************************file from parse.c********************************/
/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is include which follows the "include" declaration
** in the input file. */
#include <stdio.h>
#line 54 "parse.y"

/*
** An instance of this structure holds information about the
** LIMIT clause of a SELECT statement.
*/
struct LimitVal {
  Expr *pLimit;    /* The LIMIT expression.  NULL if there is no limit */
  Expr *pOffset;   /* The OFFSET expression.  NULL if there is none */
};

/*
** An instance of this structure is used to store the LIKE,
** GLOB, NOT LIKE, and NOT GLOB operators.
*/
struct LikeOp {
  Token eOperator;  /* "like" or "glob" or "regexp" */
  int not;         /* True if the NOT keyword is present */
};

/*
** An instance of the following structure describes the event of a
** TRIGGER.  "a" is the event type, one of TK_UPDATE, TK_INSERT,
** TK_DELETE, or TK_INSTEAD.  If the event is of the form
**
**      UPDATE ON (a,b,c)
**
** Then the "b" IdList records the list "a,b,c".
*/
struct TrigEvent { int a; IdList * b; };

/*
** An instance of this structure holds the ATTACH key and the key type.
*/
struct AttachKey { int type;  Token key; };

#line 48 "parse.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    sqlite3ParserTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is sqlite3ParserTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.
**    sqlite3ParserARG_SDECL     A static variable declaration for the %extra_argument
**    sqlite3ParserARG_PDECL     A parameter declaration for the %extra_argument
**    sqlite3ParserARG_STORE     Code to store %extra_argument into yypParser
**    sqlite3ParserARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned short int
#define YYNOCODE 253
#define YYACTIONTYPE unsigned short int
#define sqlite3ParserTOKENTYPE Token
typedef union {
  sqlite3ParserTOKENTYPE yy0;
  int yy4;
  struct {int value; int mask;} yy215;
  SrcList* yy259;
  ValuesList* yy287;
  struct LimitVal yy292;
  Expr* yy314;
  ExprList* yy322;
  struct LikeOp yy342;
  Token yy378;
  IdList* yy384;
  Select* yy387;
  int yy505;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define sqlite3ParserARG_SDECL Parse *pParse;
#define sqlite3ParserARG_PDECL ,Parse *pParse
#define sqlite3ParserARG_FETCH Parse *pParse = yypParser->pParse
#define sqlite3ParserARG_STORE yypParser->pParse = pParse
#define YYNSTATE 510
#define YYNRULE 280
#define YYERRORSYMBOL 148
#define YYERRSYMDT yy505
#define YYFALLBACK 1
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* Next are that tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   238,  257,  288,  150,  271,  373,   57,   56,   56,   56,
 /*    10 */    56,  271,   58,   58,   58,   58,   59,   59,   60,   60,
 /*    20 */    60,   61,  363,    4,  363,   31,  321,  326,   61,  315,
 /*    30 */   167,  363,   26,  386,  183,  316,  487,  488,  489,  163,
 /*    40 */    58,   58,   58,   58,   59,   59,   60,   60,   60,   61,
 /*    50 */    55,   53,  246,  394,  397,  391,  391,   57,   56,   56,
 /*    60 */    56,   56,  169,   58,   58,   58,   58,   59,   59,   60,
 /*    70 */    60,   60,   61,  238,  364,   74,  373,   72,  204,   57,
 /*    80 */   381,  172,   63,   58,   58,   58,   58,   59,   59,   60,
 /*    90 */    60,   60,   61,  242,  339,  450,  791,  106,  509,  104,
 /*   100 */    99,    1,  447,  217,  319,  242,  386,   62,  420,  173,
 /*   110 */    52,  135,   62,  408,  173,  217,  135,   17,  493,  494,
 /*   120 */   495,  496,  497,   55,   53,  246,  394,  397,  391,  391,
 /*   130 */    57,   56,   56,   56,   56,  127,   58,   58,   58,   58,
 /*   140 */    59,   59,   60,   60,   60,   61,  238,  426,  258,   94,
 /*   150 */   453,  167,   57,   59,   59,   60,   60,   60,   61,  235,
 /*   160 */   163,  343,  383,  457,  147,   62,  455,  173,  447,  135,
 /*   170 */   271,  143,  344,  193,  267,  197,  268,  154,  150,  386,
 /*   180 */   373,  133,  218,  506,  344,  193,  267,  197,  268,  154,
 /*   190 */   363,   43,  373,  395,  218,  364,   55,   53,  246,  394,
 /*   200 */   397,  391,  391,   57,   56,   56,   56,   56,  415,   58,
 /*   210 */    58,   58,   58,   59,   59,   60,   60,   60,   61,  238,
 /*   220 */   104,  315,  383,  275,  147,   57,  271,  395,  487,  488,
 /*   230 */   489,  134,  121,  121,  340,  445,  771,  125,  255,  493,
 /*   240 */   494,  495,  496,  497,   76,   19,  363,   25,  458,  320,
 /*   250 */   190,  373,  386,  373,  469,  159,   18,  318,  290,  293,
 /*   260 */   294,  313,  120,  373,  410,  373,  364,  295,  251,   55,
 /*   270 */    53,  246,  394,  397,  391,  391,   57,   56,   56,   56,
 /*   280 */    56,  138,   58,   58,   58,   58,   59,   59,   60,   60,
 /*   290 */    60,   61,  238,   60,   60,   60,   61,  271,   57,  362,
 /*   300 */   271,  486,  271,  361,  415,  341,  342,  159,  385,  271,
 /*   310 */   290,  293,  294,  216,  245,  376,  377,  363,   21,  295,
 /*   320 */   363,   30,  363,   30,  373,  386,  130,  131,  159,  363,
 /*   330 */    30,  290,  293,  294,  340,  454,  373,   68,  424,  367,
 /*   340 */   295,  152,   55,   53,  246,  394,  397,  391,  391,   57,
 /*   350 */    56,   56,   56,   56,  430,   58,   58,   58,   58,   59,
 /*   360 */    59,   60,   60,   60,   61,  249,  467,  458,  238,  369,
 /*   370 */   369,  369,  284,  338,   57,   18,  364,  404,  171,  405,
 /*   380 */   298,  340,  271,  429,    3,  385,  405,  271,  153,  140,
 /*   390 */   274,  376,  377,  287,  459,  340,  481,  256,  132,  468,
 /*   400 */   456,  386,  363,   36,  340,  341,  342,  363,   43,  451,
 /*   410 */    71,   67,  454,   69,  155,  340,  367,  485,   55,   53,
 /*   420 */   246,  394,  397,  391,  391,   57,   56,   56,   56,   56,
 /*   430 */   433,   58,   58,   58,   58,   59,   59,   60,   60,   60,
 /*   440 */    61,  350,  282,  208,  238,  117,  369,  369,  369,  400,
 /*   450 */    57,   10,  341,  342,  171,  254,  351,  401,  271,  340,
 /*   460 */   466,  271,  434,  340,  198,  239,  341,  342,  465,  218,
 /*   470 */   324,  340,  352,    1,  348,  341,  342,  386,  363,   29,
 /*   480 */   119,  363,   36,  205,  115,  349,  341,  342,  314,  278,
 /*   490 */   416,  399,  399,  139,   55,   53,  246,  394,  397,  391,
 /*   500 */   391,   57,   56,   56,   56,   56,  452,   58,   58,   58,
 /*   510 */    58,   59,   59,   60,   60,   60,   61,  387,  325,  322,
 /*   520 */   238,  510,  206,  227,  325,  322,   57,  428,  420,  271,
 /*   530 */   341,  342,  169,  358,  341,  342,  250,  389,  390,  315,
 /*   540 */   271,  359,  341,  342,  439,  232,  487,  488,  489,  363,
 /*   550 */    48,  346,  347,  386,  417,  278,  485,  399,  399,  420,
 /*   560 */   363,   22,  233,  278,  388,  399,  399,  210,  440,  424,
 /*   570 */    55,   53,  246,  394,  397,  391,  391,   57,   56,   56,
 /*   580 */    56,   56,  221,   58,   58,   58,   58,   59,   59,   60,
 /*   590 */    60,   60,   61,  271,  227,  271,  238,  271,  212,  271,
 /*   600 */   136,  271,   57,  271,  445,  271,  445,    6,  445,  285,
 /*   610 */   289,  271,  209,  363,   46,  363,   30,  363,   85,  363,
 /*   620 */    83,  363,   87,  363,   88,  363,   95,  317,  248,  386,
 /*   630 */   499,  363,   96,  473,  362,  364,  307,  181,  361,  165,
 /*   640 */   227,  252,  156,  157,  158,  307,   55,   53,  246,  394,
 /*   650 */   397,  391,  391,   57,   56,   56,   56,   56,  247,   58,
 /*   660 */    58,   58,   58,   59,   59,   60,   60,   60,   61,  238,
 /*   670 */   271,  137,  405,  271,  364,   57,  271,  445,  271,  228,
 /*   680 */   355,  170,  216,  271,  215,  164,  216,  271,   49,  356,
 /*   690 */   363,   16,  271,  363,   86,  271,  363,   47,  363,   97,
 /*   700 */   182,  499,  386,  363,   98,  227,  364,  363,   23,  310,
 /*   710 */   253,  227,  363,   32,  272,  363,   33,  500,  144,   55,
 /*   720 */    53,  246,  394,  397,  391,  391,   57,   56,   56,   56,
 /*   730 */    56,  271,   58,   58,   58,   58,   59,   59,   60,   60,
 /*   740 */    60,   61,  238,  271,  305,  271,  500,  271,   57,  271,
 /*   750 */   231,  363,   24,  271,  413,  216,  271,  161,  271,  307,
 /*   760 */   271,  498,  271,  363,   34,  363,   35,  363,   37,  363,
 /*   770 */    38,  149,  140,  363,   39,  386,  363,   27,  363,   28,
 /*   780 */   363,   40,  363,   41,  278,  110,  399,  399,   74,  500,
 /*   790 */   501,  500,  364,   53,  246,  394,  397,  391,  391,   57,
 /*   800 */    56,   56,   56,   56,  155,   58,   58,   58,   58,   59,
 /*   810 */    59,   60,   60,   60,   61,  238,  271,  500,  271,  166,
 /*   820 */   271,   57,  276,  312,  379,  379,  261,  263,  241,  192,
 /*   830 */   101,  547,  374,  503,  194,  504,  363,   42,  363,   44,
 /*   840 */   363,   45,  438,  195,  403,  443,  368,  422,  386,  129,
 /*   850 */    20,  382,  406,  414,  419,  239,  423,  214,  436,  435,
 /*   860 */   461,  505,   13,  110,  166,  151,  441,  246,  394,  397,
 /*   870 */   391,  391,   57,   56,   56,   56,   56,    5,   58,   58,
 /*   880 */    58,   58,   59,   59,   60,   60,   60,   61,    5,  480,
 /*   890 */   442,  431,   64,  279,  220,  297,  478,  244,  479,  482,
 /*   900 */   460,  222,  392,   64,  279,  225,  306,  502,  244,   13,
 /*   910 */   273,  196,  110,  110,   80,  437,   80,  151,  333,  199,
 /*   920 */   128,  273,  378,  380,  280,  277,  409,  396,  230,  491,
 /*   930 */   434,  412,  492,  444,  446,  122,  123,  329,  328,  292,
 /*   940 */   331,  385,  332,    9,  260,  264,  336,  265,  266,  345,
 /*   950 */   353,  243,  385,  354,  357,  360,  365,   74,  200,   66,
 /*   960 */    65,    5,   73,  203,  283,  201,  281,   64,  269,  270,
 /*   970 */    66,   65,  367,  202,   51,  148,   64,  279,   64,  269,
 /*   980 */   270,  244,   70,  367,  366,  407,  176,  207,  169,  410,
 /*   990 */   107,  411,  177,  175,  273,  286,   75,  211,   90,  236,
 /*  1000 */   421,  213,  369,  369,  369,  370,  371,  372,   12,  425,
 /*  1010 */   179,  180,  449,  369,  369,  369,  370,  371,  372,   12,
 /*  1020 */   462,    5,  448,    2,  237,  385,  472,  299,  463,  187,
 /*  1030 */   259,  300,  464,  188,  189,  223,   64,  279,  162,  303,
 /*  1040 */   229,  244,  470,   66,   65,  427,  474,  102,  476,  105,
 /*  1050 */    11,   64,  269,  270,  273,  111,  367,  116,  311,  187,
 /*  1060 */   259,   84,  477,  188,  189,  223,  240,  178,   89,  327,
 /*  1070 */   330,  234,  262,  103,  334,  335,  191,  102,  484,  337,
 /*  1080 */   168,  308,   50,  548,  549,  385,  369,  369,  369,  370,
 /*  1090 */   371,  372,   12,  145,  146,   54,  393,  375,  384,  174,
 /*  1100 */   398,  126,    7,   66,   65,  402,   14,    8,  484,   13,
 /*  1110 */   418,   64,  269,  270,  108,  141,  367,  109,  432,  291,
 /*  1120 */    91,  296,  219,   93,  304,  113,  483,   92,   77,  302,
 /*  1130 */   160,  195,  224,  301,  169,  114,  112,  226,  471,   15,
 /*  1140 */   185,  475,  490,  186,  507,  309,  369,  369,  369,  370,
 /*  1150 */   371,  372,   12,   78,  304,  113,  483,   79,  118,  302,
 /*  1160 */   164,   81,  142,  100,  169,  124,  323,  534,  184,   82,
 /*  1170 */   508,  534,  534,  534,  534,  534,  534,  534,  534,  534,
 /*  1180 */   534,  534,  534,  534,  534,  534,  534,  534,  534,  534,
 /*  1190 */   534,  534,  534,  100,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    16,   49,  160,   19,  160,   21,   22,   74,   75,   76,
 /*    10 */    77,  160,   79,   80,   81,   82,   83,   84,   85,   86,
 /*    20 */    87,   88,  180,  181,  180,  181,  152,  153,   88,  131,
 /*    30 */   156,  180,  181,   49,  136,  137,  138,  139,  140,  165,
 /*    40 */    79,   80,   81,   82,   83,   84,   85,   86,   87,   88,
 /*    50 */    66,   67,   68,   69,   70,   71,   72,   73,   74,   75,
 /*    60 */    76,   77,  112,   79,   80,   81,   82,   83,   84,   85,
 /*    70 */    86,   87,   88,   16,  200,  123,   92,   20,  234,   22,
 /*    80 */    83,   84,   78,   79,   80,   81,   82,   83,   84,   85,
 /*    90 */    86,   87,   88,   16,  178,  179,  149,  150,  151,  225,
 /*   100 */    23,  154,  186,   26,   14,   16,   49,  229,  172,  231,
 /*   110 */    53,  233,  229,  230,  231,   26,  233,    1,  244,  245,
 /*   120 */   246,  247,  248,   66,   67,   68,   69,   70,   71,   72,
 /*   130 */    73,   74,   75,   76,   77,   19,   79,   80,   81,   82,
 /*   140 */    83,   84,   85,   86,   87,   88,   16,  211,  153,   19,
 /*   150 */   180,  156,   22,   83,   84,   85,   86,   87,   88,  169,
 /*   160 */   165,  179,  172,  173,  174,  229,  180,  231,  186,  233,
 /*   170 */   160,   94,   95,   96,   97,   98,   99,  100,   19,   49,
 /*   180 */    21,   20,  105,   93,   95,   96,   97,   98,   99,  100,
 /*   190 */   180,  181,   21,   92,  105,  200,   66,   67,   68,   69,
 /*   200 */    70,   71,   72,   73,   74,   75,   76,   77,   20,   79,
 /*   210 */    80,   81,   82,   83,   84,   85,   86,   87,   88,   16,
 /*   220 */   225,  131,  172,  173,  174,   22,  160,  126,  138,  139,
 /*   230 */   140,  165,  142,  143,   21,  160,  146,  147,  228,  244,
 /*   240 */   245,  246,  247,  248,    1,    1,  180,  181,  160,  242,
 /*   250 */   162,   92,   49,   21,  166,   94,  168,  250,   97,   98,
 /*   260 */    99,  241,  242,   92,   21,   21,  200,  106,  193,   66,
 /*   270 */    67,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*   280 */    77,   93,   79,   80,   81,   82,   83,   84,   85,   86,
 /*   290 */    87,   88,   16,   85,   86,   87,   88,  160,   22,  109,
 /*   300 */   160,  130,  160,  113,  116,   92,   93,   94,   65,  160,
 /*   310 */    97,   98,   99,  238,  175,  176,  177,  180,  181,  106,
 /*   320 */   180,  181,  180,  181,   92,   49,   83,   84,   94,  180,
 /*   330 */   181,   97,   98,   99,   21,   92,   92,  134,  160,   96,
 /*   340 */   106,  165,   66,   67,   68,   69,   70,   71,   72,   73,
 /*   350 */    74,   75,   76,   77,    2,   79,   80,   81,   82,   83,
 /*   360 */    84,   85,   86,   87,   88,  223,  192,  160,   16,  126,
 /*   370 */   127,  128,  223,  166,   22,  168,  200,  237,  239,  237,
 /*   380 */    16,   21,  160,    2,    1,   65,  237,  160,  212,  213,
 /*   390 */   175,  176,  177,  217,  192,   21,    2,  219,  191,  192,
 /*   400 */   171,   49,  180,  181,   21,   92,   93,  180,  181,  180,
 /*   410 */   134,  133,   92,  135,   50,   21,   96,  172,   66,   67,
 /*   420 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*   430 */     2,   79,   80,   81,   82,   83,   84,   85,   86,   87,
 /*   440 */    88,   12,  220,  221,   16,   25,  126,  127,  128,    2,
 /*   450 */    22,    1,   92,   93,  239,  228,   27,    2,  160,   21,
 /*   460 */   186,  160,  188,   21,  201,  101,   92,   93,  194,  105,
 /*   470 */   151,   21,   43,  154,   45,   92,   93,   49,  180,  181,
 /*   480 */   242,  180,  181,  160,  124,   56,   92,   93,  243,  108,
 /*   490 */   116,  110,  111,  160,   66,   67,   68,   69,   70,   71,
 /*   500 */    72,   73,   74,   75,   76,   77,    2,   79,   80,   81,
 /*   510 */    82,   83,   84,   85,   86,   87,   88,   49,    3,    4,
 /*   520 */    16,    0,  221,  160,    3,    4,   22,   85,  172,  160,
 /*   530 */    92,   93,  112,   30,   92,   93,   25,   69,   70,  131,
 /*   540 */   160,   38,   92,   93,   33,  125,  138,  139,  140,  180,
 /*   550 */   181,  196,  197,   49,  116,  108,  172,  110,  111,  172,
 /*   560 */   180,  181,  199,  108,   96,  110,  111,  211,   57,  160,
 /*   570 */    66,   67,   68,   69,   70,   71,   72,   73,   74,   75,
 /*   580 */    76,   77,    2,   79,   80,   81,   82,   83,   84,   85,
 /*   590 */    86,   87,   88,  160,  160,  160,   16,  160,  211,  160,
 /*   600 */   165,  160,   22,  160,  160,  160,  160,  202,  160,  160,
 /*   610 */    85,  160,  160,  180,  181,  180,  181,  180,  181,  180,
 /*   620 */   181,  180,  181,  180,  181,  180,  181,  243,  219,   49,
 /*   630 */    21,  180,  181,  199,  109,  200,  160,  193,  113,  165,
 /*   640 */   160,  193,  102,  103,  104,  160,   66,   67,   68,   69,
 /*   650 */    70,   71,   72,   73,   74,   75,   76,   77,  223,   79,
 /*   660 */    80,   81,   82,   83,   84,   85,   86,   87,   88,   16,
 /*   670 */   160,  165,  237,  160,  200,   22,  160,  160,  160,  199,
 /*   680 */    37,  203,  238,  160,  238,   20,  238,  160,  210,   46,
 /*   690 */   180,  181,  160,  180,  181,  160,  180,  181,  180,  181,
 /*   700 */   224,   92,   49,  180,  181,  160,  200,  180,  181,  224,
 /*   710 */   193,  160,  180,  181,  160,  180,  181,  205,    1,   66,
 /*   720 */    67,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*   730 */    77,  160,   79,   80,   81,   82,   83,   84,   85,   86,
 /*   740 */    87,   88,   16,  160,  199,  160,  205,  160,   22,  160,
 /*   750 */   199,  180,  181,  160,   20,  238,  160,  165,  160,  160,
 /*   760 */   160,  249,  160,  180,  181,  180,  181,  180,  181,  180,
 /*   770 */   181,  212,  213,  180,  181,   49,  180,  181,  180,  181,
 /*   780 */   180,  181,  180,  181,  108,   20,  110,  111,  123,  205,
 /*   790 */   249,  205,  200,   67,   68,   69,   70,   71,   72,   73,
 /*   800 */    74,   75,   76,   77,   50,   79,   80,   81,   82,   83,
 /*   810 */    84,   85,   86,   87,   88,   16,  160,  205,  160,   70,
 /*   820 */   160,   22,   16,  224,  126,  127,   21,   24,   23,   26,
 /*   830 */   160,  114,    2,  249,   96,  249,  180,  181,  180,  181,
 /*   840 */   180,  181,   23,  105,    2,    2,  160,    2,   49,  115,
 /*   850 */    20,  172,  160,  214,  160,  101,  160,  160,  189,  160,
 /*   860 */    95,  249,   20,   20,  115,   20,  189,   68,   69,   70,
 /*   870 */    71,   72,   73,   74,   75,   76,   77,    1,   79,   80,
 /*   880 */    81,   82,   83,   84,   85,   86,   87,   88,    1,    2,
 /*   890 */   189,    2,   16,   17,    2,    2,    2,   21,    2,    2,
 /*   900 */   160,  160,   96,   16,   17,  160,  160,  160,   21,   20,
 /*   910 */    34,  183,   20,   20,   20,   96,   20,   20,  157,  204,
 /*   920 */   202,   34,  240,  240,  236,  232,  183,  232,  205,  172,
 /*   930 */   188,  183,  172,  183,  183,  251,  251,   10,  155,  187,
 /*   940 */   155,   65,  155,   20,  163,  167,   22,  167,  101,  182,
 /*   950 */   182,   47,   65,  190,  182,  182,  200,  123,  205,   83,
 /*   960 */    84,    1,  121,  208,  120,  206,  118,   91,   92,   93,
 /*   970 */    83,   84,   96,  207,  122,  114,   16,   17,   91,   92,
 /*   980 */    93,   21,  133,   96,  209,  161,  222,   26,  112,   21,
 /*   990 */   161,  171,  222,  235,   34,  117,  101,  215,    1,  184,
 /*  1000 */   216,  215,  126,  127,  128,  129,  130,  131,  132,  216,
 /*  1010 */   222,  222,  190,  126,  127,  128,  129,  130,  131,  132,
 /*  1020 */   182,    1,  171,    1,  184,   65,   18,  158,  182,    7,
 /*  1030 */     8,   15,  182,   11,   12,   13,   16,   17,  159,   15,
 /*  1040 */   205,   21,  161,   83,   84,   85,  161,   25,  209,    1,
 /*  1050 */     1,   91,   92,   93,   34,  198,   96,  226,  226,    7,
 /*  1060 */     8,  170,  209,   11,   12,   13,  164,  185,  185,    2,
 /*  1070 */     9,    2,   22,   35,   21,   21,   25,   25,   56,   21,
 /*  1080 */    51,  227,    1,  114,  114,   65,  126,  127,  128,  129,
 /*  1090 */   130,  131,  132,  114,  114,    1,   96,    2,    2,   67,
 /*  1100 */   109,    1,  119,   83,   84,   41,   20,  119,   56,   20,
 /*  1110 */   116,   91,   92,   93,    1,  114,   96,    2,    2,   51,
 /*  1120 */     1,   51,    2,    1,  102,  103,  104,    1,    1,  107,
 /*  1130 */   100,  105,   14,   16,  112,   25,  115,   14,   17,    1,
 /*  1140 */   144,   17,  114,  145,    3,   20,  126,  127,  128,  129,
 /*  1150 */   130,  131,  132,   22,  102,  103,  104,   22,   25,  107,
 /*  1160 */    20,   22,   20,  141,  112,  146,    5,  252,   25,   22,
 /*  1170 */     6,  252,  252,  252,  252,  252,  252,  252,  252,  252,
 /*  1180 */   252,  252,  252,  252,  252,  252,  252,  252,  252,  252,
 /*  1190 */   252,  252,  252,  141,
};
#define YY_SHIFT_USE_DFLT (-103)
#define YY_SHIFT_MAX 323
static const short yy_shift_ofst[] = {
 /*     0 */   515, 1022, 1052,  876,  -16,  876, 1020, 1020, 1020,  213,
 /*    10 */   -50,  887, 1020, 1020, 1020, 1020,    4,  313,  232,   -3,
 /*    20 */    -3,   57,  130,  203,  276,  352,  428,  504,  580,  653,
 /*    30 */   653,  653,  653,  653,  653,  653,  653,  653,  653,  653,
 /*    40 */   653,  653,  653,  653,  653,  653,  726,  799,  799,  960,
 /*    50 */  1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020,
 /*    60 */  1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020,
 /*    70 */  1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020,
 /*    80 */  1020, 1020, 1020,  -67,   77,  -39,  -39,   70,  208,  364,
 /*    90 */   313,  313,  313,  313,  232,  -60, -103, -103, -103,  243,
 /*   100 */    90,   89, -102,  429,  360,  394,  521,  159,  313,  159,
 /*   110 */   313,  313,  313,  313,  313,  313,  420,  313,  313,  171,
 /*   120 */   171,  -48,  -48,  -48,  -48,  -48,  -50,  -50, -103, -103,
 /*   130 */   320,  320,  161,  234,  381,  383,  447,  455,  374,  438,
 /*   140 */   450,  442,  408,  313,  525,  313,  313,  244,  313,  188,
 /*   150 */   313,  313,  676,  188,  313,  313,  511,  511,  511,  313,
 /*   160 */   313,  676,  313,  676,  313,  676,  313,  116,  503,  190,
 /*   170 */   734,  698,  698,  101,  101,  278,  503,  503,  754,  503,
 /*   180 */   503,  765,  665,  232,  232,  749,  749,  927,  927,  927,
 /*   190 */   923,  924,  924,  847,  847,  904,  847,  847,  -50,  834,
 /*   200 */   844,  841,  848,  852,  849,  861,  961,  968,  961,  861,
 /*   210 */   895,  878,  895,  878,  997,  961,  961,  968,  904,  847,
 /*   220 */   847,  847,  997, 1008, 1016,  861, 1024,  861,  834,  852,
 /*   230 */   852, 1048, 1049, 1048, -103, -103, -103, -103,  468,  540,
 /*   240 */   805,  803,  738,  643,  717,  830,  806,  842,  845,  889,
 /*   250 */   819,  843,  892,  893,  894,  896,  897,  609, 1067, 1061,
 /*   260 */  1069, 1050, 1053, 1051, 1054, 1058, 1038, 1029, 1081,  969,
 /*   270 */   970,  979,  980, 1094, 1095, 1096, 1000, 1032,  991, 1100,
 /*   280 */  1064,  983, 1086,  988, 1089,  994, 1113, 1115, 1001, 1116,
 /*   290 */  1068, 1119, 1120, 1126, 1127, 1070, 1122, 1030, 1026, 1118,
 /*   300 */  1117, 1121, 1123, 1124, 1021, 1110, 1131, 1135, 1125, 1138,
 /*   310 */  1140, 1133, 1140, 1142, 1139, 1028, 1143, 1147,  996,  998,
 /*   320 */  1019, 1141, 1161, 1164,
};
#define YY_REDUCE_USE_DFLT (-159)
#define YY_REDUCE_MAX 237
static const short yy_reduce_ofst[] = {
 /*     0 */   -53, -126,   -5,  435,  -64,   66,  142,  222,  149,  207,
 /*    10 */   176,   10, -156,  140,  301,  227, -117,   88,  -10,  139,
 /*    20 */   215, -122, -122, -122, -122, -122, -122, -122, -122, -122,
 /*    30 */  -122, -122, -122, -122, -122, -122, -122, -122, -122, -122,
 /*    40 */  -122, -122, -122, -122, -122, -122, -122, -122, -122, -158,
 /*    50 */  -149,  137,  298,  369,  380,  433,  437,  439,  441,  443,
 /*    60 */   445,  451,  510,  513,  516,  518,  523,  527,  532,  535,
 /*    70 */   571,  583,  585,  587,  589,  593,  596,  598,  600,  602,
 /*    80 */   656,  658,  660, -122,  -84, -122, -122, -122, -122,  274,
 /*    90 */    75,  444,  448,  517,   50, -122, -122, -122, -122,  229,
 /*   100 */     7,  -18,   20,  355,  363,  178,  319,  356,  409,  387,
 /*   110 */   446,  434,  480,  545,  476,  551,  474,  485,  599,  245,
 /*   120 */   384,  512,  541,  584,  586,  612,  506,  592,  478,  559,
 /*   130 */   -30,  -14,  174,  202,  263,  323,  263,  263,  333,  449,
 /*   140 */   452,  554,  238,  670,  405,  554,  686,  679,  692,  639,
 /*   150 */   694,  696,  263,  639,  697,  699,  669,  677,  701,  740,
 /*   160 */   741,  263,  745,  263,  746,  263,  747,  761,  728,  718,
 /*   170 */   715,  682,  683,  693,  695,  688,  743,  748,  742,  750,
 /*   180 */   751,  752,  723,  757,  760,  684,  685,  783,  785,  787,
 /*   190 */   781,  778,  780,  767,  768,  763,  772,  773,  756,  753,
 /*   200 */   759,  766,  755,  775,  758,  824,  764,  820,  770,  829,
 /*   210 */   782,  784,  786,  793,  815,  788,  789,  851,  822,  838,
 /*   220 */   846,  850,  840,  869,  879,  881,  857,  885,  835,  839,
 /*   230 */   853,  831,  854,  832,  902,  891,  882,  883,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   517,  790,  790,  748,  631,  790,  748,  790,  748,  790,
 /*    10 */   635,  790,  744,  748,  790,  790,  718,  790,  550,  758,
 /*    20 */   758,  666,  790,  790,  790,  790,  790,  790,  790,  667,
 /*    30 */   747,  743,  739,  741,  740,  668,  655,  664,  671,  647,
 /*    40 */   673,  674,  684,  685,  765,  766,  708,  724,  707,  790,
 /*    50 */   790,  790,  790,  790,  790,  790,  790,  790,  790,  790,
 /*    60 */   790,  790,  790,  790,  790,  790,  790,  790,  790,  790,
 /*    70 */   790,  790,  790,  790,  790,  790,  790,  790,  790,  790,
 /*    80 */   790,  790,  790,  710,  543,  709,  717,  711,  712,  604,
 /*    90 */   790,  790,  790,  790,  790,  713,  714,  725,  726,  790,
 /*   100 */   785,  790,  771,  790,  790,  790,  517,  631,  790,  631,
 /*   110 */   790,  790,  790,  790,  790,  790,  790,  790,  790,  790,
 /*   120 */   790,  670,  670,  670,  670,  670,  790,  790,  625,  635,
 /*   130 */   790,  790,  595,  790,  790,  790,  790,  790,  790,  790,
 /*   140 */   790,  790,  771,  790,  623,  790,  790,  552,  790,  633,
 /*   150 */   790,  790,  638,  639,  790,  790,  790,  790,  790,  790,
 /*   160 */   790,  533,  790,  614,  790,  678,  790,  790,  658,  623,
 /*   170 */   632,  790,  790,  790,  790,  742,  658,  658,  574,  658,
 /*   180 */   658,  577,  670,  790,  790,  788,  788,  522,  522,  522,
 /*   190 */   594,  541,  541,  606,  606,  591,  606,  606,  790,  670,
 /*   200 */   661,  663,  651,  665,  790,  640,  659,  790,  659,  640,
 /*   210 */   648,  650,  648,  650,  749,  659,  659,  790,  591,  606,
 /*   220 */   606,  606,  749,  531,  528,  640,  613,  640,  670,  665,
 /*   230 */   665,  686,  790,  686,  536,  560,  579,  579,  790,  790,
 /*   240 */   532,  790,  790,  790,  694,  790,  790,  790,  790,  790,
 /*   250 */   790,  790,  790,  790,  790,  790,  790,  790,  790,  790,
 /*   260 */   790,  537,  790,  790,  790,  790,  790,  790,  790,  699,
 /*   270 */   695,  790,  696,  790,  790,  790,  790,  790,  617,  790,
 /*   280 */   790,  790,  652,  790,  662,  790,  790,  790,  790,  790,
 /*   290 */   790,  790,  790,  790,  790,  790,  790,  790,  790,  790,
 /*   300 */   790,  790,  790,  790,  790,  790,  790,  790,  675,  790,
 /*   310 */   676,  790,  677,  762,  790,  790,  790,  790,  790,  790,
 /*   320 */   790,  790,  518,  790,  511,  515,  513,  514,  520,  523,
 /*   330 */   521,  524,  525,  526,  538,  539,  542,  540,  534,  559,
 /*   340 */   547,  548,  549,  561,  568,  569,  607,  608,  609,  610,
 /*   350 */   759,  760,  761,  570,  589,  592,  593,  571,  656,  657,
 /*   360 */   572,  621,  622,  691,  615,  616,  620,  693,  697,  698,
 /*   370 */   700,  701,  702,  546,  553,  554,  557,  558,  754,  756,
 /*   380 */   755,  757,  556,  555,  703,  706,  715,  716,  722,  728,
 /*   390 */   732,  720,  721,  723,  727,  729,  730,  731,  618,  619,
 /*   400 */   735,  737,  738,  733,  745,  746,  641,  736,  719,  653,
 /*   410 */   545,  660,  654,  624,  634,  643,  644,  645,  646,  629,
 /*   420 */   630,  636,  649,  689,  690,  637,  626,  627,  628,  734,
 /*   430 */   692,  704,  705,  573,  580,  581,  582,  585,  586,  587,
 /*   440 */   588,  583,  584,  750,  751,  753,  752,  575,  576,  590,
 /*   450 */   562,  563,  564,  565,  699,  566,  567,  551,  544,  596,
 /*   460 */   599,  578,  600,  601,  602,  603,  605,  597,  598,  535,
 /*   470 */   527,  529,  530,  611,  642,  612,  669,  672,  681,  682,
 /*   480 */   683,  687,  688,  679,  680,  772,  773,  767,  768,  769,
 /*   490 */   770,  763,  764,  774,  775,  776,  777,  778,  779,  786,
 /*   500 */   787,  780,  789,  781,  782,  783,  784,  516,  519,  512,
};
#define YY_SZ_ACTTAB (sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammer, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
    0,  /*          $ => nothing */
    0,  /*         LP => nothing */
    0,  /*         RP => nothing */
    0,  /*       SEMI => nothing */
   21,  /*    EXPLAIN => ID */
   21,  /*      QUERY => ID */
   21,  /*       PLAN => ID */
   21,  /*      BEGIN => ID */
    0,  /*      START => nothing */
    0,  /* TRANSACTION => nothing */
    0,  /*       WORK => nothing */
    0,  /*     COMMIT => nothing */
    0,  /*   ROLLBACK => nothing */
    0,  /*     CREATE => nothing */
    0,  /*      TABLE => nothing */
   21,  /*         IF => ID */
    0,  /*        NOT => nothing */
    0,  /*     EXISTS => nothing */
   21,  /*       TEMP => ID */
    0,  /*         AS => nothing */
    0,  /*      COMMA => nothing */
    0,  /*         ID => nothing */
    0,  /*         EQ => nothing */
    0,  /*    DEFAULT => nothing */
    0,  /*    CHARSET => nothing */
    0,  /*        SET => nothing */
    0,  /*    COLLATE => nothing */
   21,  /*      ABORT => ID */
   21,  /*      AFTER => ID */
   21,  /*    ANALYZE => ID */
   21,  /*        ASC => ID */
   21,  /*     ATTACH => ID */
   21,  /*     BEFORE => ID */
   21,  /*    CASCADE => ID */
   21,  /*       CAST => ID */
   21,  /*   CONFLICT => ID */
   21,  /*   DATABASE => ID */
   21,  /*   DEFERRED => ID */
   21,  /*       DESC => ID */
   21,  /*     DETACH => ID */
   21,  /*       EACH => ID */
   21,  /*        END => ID */
   21,  /*  EXCLUSIVE => ID */
   21,  /*       FAIL => ID */
   21,  /*        FOR => ID */
   21,  /*     IGNORE => ID */
   21,  /*  IMMEDIATE => ID */
   21,  /*  INITIALLY => ID */
   21,  /*    INSTEAD => ID */
   21,  /*    LIKE_KW => ID */
   21,  /*      MATCH => ID */
   21,  /*        KEY => ID */
   21,  /*         OF => ID */
   21,  /*     OFFSET => ID */
   21,  /*     PRAGMA => ID */
   21,  /*      RAISE => ID */
   21,  /*    REPLACE => ID */
   21,  /*   RESTRICT => ID */
   21,  /*        ROW => ID */
   21,  /*  STATEMENT => ID */
   21,  /*    TRIGGER => ID */
   21,  /*     VACUUM => ID */
   21,  /*       VIEW => ID */
   21,  /*    REINDEX => ID */
   21,  /*     RENAME => ID */
   21,  /*   CTIME_KW => ID */
    0,  /*         OR => nothing */
    0,  /*        AND => nothing */
    0,  /*         IS => nothing */
    0,  /*    BETWEEN => nothing */
    0,  /*         IN => nothing */
    0,  /*     ISNULL => nothing */
    0,  /*    NOTNULL => nothing */
    0,  /*         NE => nothing */
    0,  /*         GT => nothing */
    0,  /*         LE => nothing */
    0,  /*         LT => nothing */
    0,  /*         GE => nothing */
    0,  /*     ESCAPE => nothing */
    0,  /*     BITAND => nothing */
    0,  /*      BITOR => nothing */
    0,  /*     LSHIFT => nothing */
    0,  /*     RSHIFT => nothing */
    0,  /*       PLUS => nothing */
    0,  /*      MINUS => nothing */
    0,  /*       STAR => nothing */
    0,  /*      SLASH => nothing */
    0,  /*        REM => nothing */
    0,  /*     CONCAT => nothing */
    0,  /*     UMINUS => nothing */
    0,  /*      UPLUS => nothing */
    0,  /*     BITNOT => nothing */
    0,  /*     STRING => nothing */
    0,  /*    JOIN_KW => nothing */
    0,  /* CONSTRAINT => nothing */
    0,  /*   AUTOINCR => nothing */
    0,  /*       NULL => nothing */
    0,  /*    PRIMARY => nothing */
    0,  /*     UNIQUE => nothing */
    0,  /*      CHECK => nothing */
    0,  /* REFERENCES => nothing */
    0,  /*         ON => nothing */
    0,  /*     DELETE => nothing */
    0,  /*     UPDATE => nothing */
    0,  /*     INSERT => nothing */
    0,  /* DEFERRABLE => nothing */
    0,  /*    FOREIGN => nothing */
    0,  /*       DROP => nothing */
    0,  /*      UNION => nothing */
    0,  /*        ALL => nothing */
    0,  /*     EXCEPT => nothing */
    0,  /*  INTERSECT => nothing */
    0,  /*     SELECT => nothing */
    0,  /*   DISTINCT => nothing */
    0,  /*        DOT => nothing */
    0,  /*       FROM => nothing */
    0,  /*       JOIN => nothing */
    0,  /*      USING => nothing */
    0,  /*      ORDER => nothing */
    0,  /*         BY => nothing */
    0,  /*      GROUP => nothing */
    0,  /*     HAVING => nothing */
    0,  /*      LIMIT => nothing */
    0,  /*      WHERE => nothing */
    0,  /*       INTO => nothing */
    0,  /*     VALUES => nothing */
    0,  /*    INTEGER => nothing */
    0,  /*      FLOAT => nothing */
    0,  /*       BLOB => nothing */
    0,  /*   REGISTER => nothing */
    0,  /*   VARIABLE => nothing */
    0,  /*  VARIABLE1 => nothing */
    0,  /*       CASE => nothing */
    0,  /*       WHEN => nothing */
    0,  /*       THEN => nothing */
    0,  /*       ELSE => nothing */
    0,  /*      NAMES => nothing */
    0,  /*  CHARACTER => nothing */
    0,  /*     GLOBAL => nothing */
    0,  /*      LOCAL => nothing */
    0,  /*    SESSION => nothing */
    0,  /*       SHOW => nothing */
    0,  /*  DATABASES => nothing */
    0,  /*    SCHEMAS => nothing */
    0,  /*     TABLES => nothing */
    0,  /*     STATUS => nothing */
    0,  /*  VARIABLES => nothing */
    0,  /*  COLLATION => nothing */
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  int stateno;       /* The state-number */
  int major;         /* The major token value.  This is the code
                     ** number for the token at this stack level */
  YYMINORTYPE minor; /* The user-supplied minor token value.  This
                     ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
  int yyerrcnt;                 /* Shifts left before out of the error */
  sqlite3ParserARG_SDECL                /* A place to hold %extra_argument */
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void sqlite3ParserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "LP",            "RP",            "SEMI",        
  "EXPLAIN",       "QUERY",         "PLAN",          "BEGIN",       
  "START",         "TRANSACTION",   "WORK",          "COMMIT",      
  "ROLLBACK",      "CREATE",        "TABLE",         "IF",          
  "NOT",           "EXISTS",        "TEMP",          "AS",          
  "COMMA",         "ID",            "EQ",            "DEFAULT",     
  "CHARSET",       "SET",           "COLLATE",       "ABORT",       
  "AFTER",         "ANALYZE",       "ASC",           "ATTACH",      
  "BEFORE",        "CASCADE",       "CAST",          "CONFLICT",    
  "DATABASE",      "DEFERRED",      "DESC",          "DETACH",      
  "EACH",          "END",           "EXCLUSIVE",     "FAIL",        
  "FOR",           "IGNORE",        "IMMEDIATE",     "INITIALLY",   
  "INSTEAD",       "LIKE_KW",       "MATCH",         "KEY",         
  "OF",            "OFFSET",        "PRAGMA",        "RAISE",       
  "REPLACE",       "RESTRICT",      "ROW",           "STATEMENT",   
  "TRIGGER",       "VACUUM",        "VIEW",          "REINDEX",     
  "RENAME",        "CTIME_KW",      "OR",            "AND",         
  "IS",            "BETWEEN",       "IN",            "ISNULL",      
  "NOTNULL",       "NE",            "GT",            "LE",          
  "LT",            "GE",            "ESCAPE",        "BITAND",      
  "BITOR",         "LSHIFT",        "RSHIFT",        "PLUS",        
  "MINUS",         "STAR",          "SLASH",         "REM",         
  "CONCAT",        "UMINUS",        "UPLUS",         "BITNOT",      
  "STRING",        "JOIN_KW",       "CONSTRAINT",    "AUTOINCR",    
  "NULL",          "PRIMARY",       "UNIQUE",        "CHECK",       
  "REFERENCES",    "ON",            "DELETE",        "UPDATE",      
  "INSERT",        "DEFERRABLE",    "FOREIGN",       "DROP",        
  "UNION",         "ALL",           "EXCEPT",        "INTERSECT",   
  "SELECT",        "DISTINCT",      "DOT",           "FROM",        
  "JOIN",          "USING",         "ORDER",         "BY",          
  "GROUP",         "HAVING",        "LIMIT",         "WHERE",       
  "INTO",          "VALUES",        "INTEGER",       "FLOAT",       
  "BLOB",          "REGISTER",      "VARIABLE",      "VARIABLE1",   
  "CASE",          "WHEN",          "THEN",          "ELSE",        
  "NAMES",         "CHARACTER",     "GLOBAL",        "LOCAL",       
  "SESSION",       "SHOW",          "DATABASES",     "SCHEMAS",     
  "TABLES",        "STATUS",        "VARIABLES",     "COLLATION",   
  "error",         "input",         "cmdlist",       "ecmd",        
  "cmdx",          "cmd",           "explain",       "trans_opt",   
  "create_table",  "create_table_args",  "temp",          "ifnotexists", 
  "nm",            "dbnm",          "columnlist",    "conslist_opt",
  "table_opt",     "select",        "column",        "eq_or_null",  
  "columnid",      "type",          "carglist",      "id",          
  "ids",           "typetoken",     "typename",      "signed",      
  "plus_num",      "minus_num",     "carg",          "ccons",       
  "term",          "expr",          "onconf",        "sortorder",   
  "idxlist_opt",   "refargs",       "defer_subclause",  "autoinc",     
  "refarg",        "refact",        "init_deferred_pred_opt",  "conslist",    
  "tcons",         "idxlist",       "defer_subclause_opt",  "orconf",      
  "resolvetype",   "raisetype",     "ifexists",      "fullname",    
  "oneselect",     "multiselect_op",  "distinct",      "selcollist",  
  "from",          "where_opt",     "groupby_opt",   "having_opt",  
  "orderby_opt",   "limit_opt",     "sclp",          "as",          
  "seltablist",    "stl_prefix",    "joinop",        "on_opt",      
  "using_opt",     "seltablist_paren",  "joinop2",       "inscollist",  
  "sortlist",      "sortitem",      "collate",       "exprlist",    
  "setlist",       "insert_cmd",    "inscollist_opt",  "valueslist",  
  "itemlist",      "likeop",        "escape",        "between_op",  
  "between_elem",  "in_op",         "case_operand",  "case_exprlist",
  "case_else",     "expritem",      "idxitem",       "plus_opt",    
  "number",        "variable_assignment_list",  "scope_qualifier",  "user_var_name",
  "show_databes",  "show_tables",   "show_table_status",  "show_variables",
  "show_collation",  "show_statement_pattern",  "full_keyword",  "from_db",     
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "input ::= cmdlist",
 /*   1 */ "cmdlist ::= cmdlist ecmd",
 /*   2 */ "cmdlist ::= ecmd",
 /*   3 */ "cmdx ::= cmd",
 /*   4 */ "cmdx ::= LP cmd RP",
 /*   5 */ "ecmd ::= SEMI",
 /*   6 */ "ecmd ::= explain cmdx SEMI",
 /*   7 */ "explain ::=",
 /*   8 */ "explain ::= EXPLAIN",
 /*   9 */ "explain ::= EXPLAIN QUERY PLAN",
 /*  10 */ "cmd ::= BEGIN trans_opt",
 /*  11 */ "cmd ::= START TRANSACTION",
 /*  12 */ "trans_opt ::=",
 /*  13 */ "trans_opt ::= WORK",
 /*  14 */ "cmd ::= COMMIT trans_opt",
 /*  15 */ "cmd ::= ROLLBACK trans_opt",
 /*  16 */ "cmd ::= create_table create_table_args",
 /*  17 */ "create_table ::= CREATE temp TABLE ifnotexists nm dbnm",
 /*  18 */ "ifnotexists ::=",
 /*  19 */ "ifnotexists ::= IF NOT EXISTS",
 /*  20 */ "temp ::= TEMP",
 /*  21 */ "temp ::=",
 /*  22 */ "create_table_args ::= LP columnlist conslist_opt RP table_opt",
 /*  23 */ "create_table_args ::= AS select",
 /*  24 */ "columnlist ::= columnlist COMMA column",
 /*  25 */ "columnlist ::= column",
 /*  26 */ "table_opt ::=",
 /*  27 */ "table_opt ::= table_opt ID",
 /*  28 */ "table_opt ::= table_opt ID EQ ID",
 /*  29 */ "table_opt ::= table_opt DEFAULT CHARSET SET eq_or_null ID",
 /*  30 */ "table_opt ::= table_opt DEFAULT COLLATE eq_or_null ID",
 /*  31 */ "eq_or_null ::=",
 /*  32 */ "eq_or_null ::= EQ",
 /*  33 */ "column ::= columnid type carglist",
 /*  34 */ "columnid ::= nm",
 /*  35 */ "id ::= ID",
 /*  36 */ "ids ::= ID|STRING",
 /*  37 */ "nm ::= ID",
 /*  38 */ "nm ::= STRING",
 /*  39 */ "nm ::= JOIN_KW",
 /*  40 */ "type ::=",
 /*  41 */ "type ::= typetoken",
 /*  42 */ "typetoken ::= typename",
 /*  43 */ "typetoken ::= typename LP signed RP",
 /*  44 */ "typetoken ::= typename LP signed COMMA signed RP",
 /*  45 */ "typename ::= ids",
 /*  46 */ "typename ::= typename ids",
 /*  47 */ "signed ::= plus_num",
 /*  48 */ "signed ::= minus_num",
 /*  49 */ "carglist ::= carglist carg",
 /*  50 */ "carglist ::=",
 /*  51 */ "carg ::= CONSTRAINT nm ccons",
 /*  52 */ "carg ::= ccons",
 /*  53 */ "carg ::= DEFAULT term",
 /*  54 */ "carg ::= DEFAULT LP expr RP",
 /*  55 */ "carg ::= DEFAULT PLUS term",
 /*  56 */ "carg ::= DEFAULT MINUS term",
 /*  57 */ "carg ::= DEFAULT id",
 /*  58 */ "ccons ::= AUTOINCR",
 /*  59 */ "ccons ::= NULL onconf",
 /*  60 */ "ccons ::= NOT NULL onconf",
 /*  61 */ "ccons ::= PRIMARY KEY sortorder onconf",
 /*  62 */ "ccons ::= UNIQUE onconf",
 /*  63 */ "ccons ::= CHECK LP expr RP",
 /*  64 */ "ccons ::= REFERENCES nm idxlist_opt refargs",
 /*  65 */ "ccons ::= defer_subclause",
 /*  66 */ "ccons ::= COLLATE id",
 /*  67 */ "autoinc ::=",
 /*  68 */ "autoinc ::= AUTOINCR",
 /*  69 */ "refargs ::=",
 /*  70 */ "refargs ::= refargs refarg",
 /*  71 */ "refarg ::= MATCH nm",
 /*  72 */ "refarg ::= ON DELETE refact",
 /*  73 */ "refarg ::= ON UPDATE refact",
 /*  74 */ "refarg ::= ON INSERT refact",
 /*  75 */ "refact ::= SET NULL",
 /*  76 */ "refact ::= SET DEFAULT",
 /*  77 */ "refact ::= CASCADE",
 /*  78 */ "refact ::= RESTRICT",
 /*  79 */ "defer_subclause ::= NOT DEFERRABLE init_deferred_pred_opt",
 /*  80 */ "defer_subclause ::= DEFERRABLE init_deferred_pred_opt",
 /*  81 */ "init_deferred_pred_opt ::=",
 /*  82 */ "init_deferred_pred_opt ::= INITIALLY DEFERRED",
 /*  83 */ "init_deferred_pred_opt ::= INITIALLY IMMEDIATE",
 /*  84 */ "conslist_opt ::=",
 /*  85 */ "conslist_opt ::= COMMA conslist",
 /*  86 */ "conslist ::= conslist COMMA tcons",
 /*  87 */ "conslist ::= conslist tcons",
 /*  88 */ "conslist ::= tcons",
 /*  89 */ "tcons ::= CONSTRAINT nm",
 /*  90 */ "tcons ::= PRIMARY KEY LP idxlist autoinc RP onconf",
 /*  91 */ "tcons ::= UNIQUE LP idxlist RP onconf",
 /*  92 */ "tcons ::= CHECK LP expr RP onconf",
 /*  93 */ "tcons ::= FOREIGN KEY LP idxlist RP REFERENCES nm idxlist_opt refargs defer_subclause_opt",
 /*  94 */ "defer_subclause_opt ::=",
 /*  95 */ "defer_subclause_opt ::= defer_subclause",
 /*  96 */ "onconf ::=",
 /*  97 */ "onconf ::= ON CONFLICT resolvetype",
 /*  98 */ "resolvetype ::= raisetype",
 /*  99 */ "resolvetype ::= IGNORE",
 /* 100 */ "resolvetype ::= REPLACE",
 /* 101 */ "cmd ::= DROP TABLE ifexists fullname",
 /* 102 */ "ifexists ::= IF EXISTS",
 /* 103 */ "ifexists ::=",
 /* 104 */ "cmd ::= select",
 /* 105 */ "select ::= oneselect",
 /* 106 */ "select ::= select multiselect_op oneselect",
 /* 107 */ "multiselect_op ::= UNION",
 /* 108 */ "multiselect_op ::= UNION ALL",
 /* 109 */ "multiselect_op ::= EXCEPT|INTERSECT",
 /* 110 */ "oneselect ::= SELECT distinct selcollist from where_opt groupby_opt having_opt orderby_opt limit_opt",
 /* 111 */ "distinct ::= DISTINCT",
 /* 112 */ "distinct ::= ALL",
 /* 113 */ "distinct ::=",
 /* 114 */ "sclp ::= selcollist COMMA",
 /* 115 */ "sclp ::=",
 /* 116 */ "selcollist ::= sclp expr as",
 /* 117 */ "selcollist ::= sclp STAR",
 /* 118 */ "selcollist ::= sclp nm DOT STAR",
 /* 119 */ "as ::= AS nm",
 /* 120 */ "as ::= ids",
 /* 121 */ "as ::=",
 /* 122 */ "from ::=",
 /* 123 */ "from ::= FROM seltablist",
 /* 124 */ "stl_prefix ::= seltablist joinop",
 /* 125 */ "stl_prefix ::=",
 /* 126 */ "seltablist ::= stl_prefix nm dbnm as on_opt using_opt",
 /* 127 */ "seltablist ::= stl_prefix LP seltablist_paren RP as on_opt using_opt",
 /* 128 */ "seltablist_paren ::= select",
 /* 129 */ "seltablist_paren ::= seltablist",
 /* 130 */ "dbnm ::=",
 /* 131 */ "dbnm ::= DOT nm",
 /* 132 */ "fullname ::= nm dbnm",
 /* 133 */ "joinop ::= COMMA|JOIN",
 /* 134 */ "joinop ::= JOIN_KW JOIN",
 /* 135 */ "joinop ::= JOIN_KW nm JOIN",
 /* 136 */ "joinop ::= JOIN_KW nm nm JOIN",
 /* 137 */ "on_opt ::= ON expr",
 /* 138 */ "on_opt ::=",
 /* 139 */ "using_opt ::= USING LP inscollist RP",
 /* 140 */ "using_opt ::=",
 /* 141 */ "orderby_opt ::=",
 /* 142 */ "orderby_opt ::= ORDER BY sortlist",
 /* 143 */ "sortlist ::= sortlist COMMA sortitem collate sortorder",
 /* 144 */ "sortlist ::= sortitem collate sortorder",
 /* 145 */ "sortitem ::= expr",
 /* 146 */ "sortorder ::= ASC",
 /* 147 */ "sortorder ::= DESC",
 /* 148 */ "sortorder ::=",
 /* 149 */ "collate ::=",
 /* 150 */ "collate ::= COLLATE id",
 /* 151 */ "groupby_opt ::=",
 /* 152 */ "groupby_opt ::= GROUP BY exprlist",
 /* 153 */ "having_opt ::=",
 /* 154 */ "having_opt ::= HAVING expr",
 /* 155 */ "limit_opt ::=",
 /* 156 */ "limit_opt ::= LIMIT expr",
 /* 157 */ "limit_opt ::= LIMIT expr OFFSET expr",
 /* 158 */ "limit_opt ::= LIMIT expr COMMA expr",
 /* 159 */ "cmd ::= DELETE FROM fullname where_opt limit_opt",
 /* 160 */ "where_opt ::=",
 /* 161 */ "where_opt ::= WHERE expr",
 /* 162 */ "cmd ::= UPDATE fullname SET setlist where_opt limit_opt",
 /* 163 */ "setlist ::= setlist COMMA nm EQ expr",
 /* 164 */ "setlist ::= nm EQ expr",
 /* 165 */ "cmd ::= insert_cmd INTO fullname inscollist_opt VALUES valueslist",
 /* 166 */ "cmd ::= insert_cmd INTO fullname inscollist_opt SET setlist",
 /* 167 */ "cmd ::= insert_cmd fullname inscollist_opt SET setlist",
 /* 168 */ "cmd ::= insert_cmd INTO fullname inscollist_opt select",
 /* 169 */ "insert_cmd ::= INSERT",
 /* 170 */ "insert_cmd ::= REPLACE",
 /* 171 */ "valueslist ::= valueslist COMMA LP itemlist RP",
 /* 172 */ "valueslist ::= LP itemlist RP",
 /* 173 */ "valueslist ::= LP RP",
 /* 174 */ "itemlist ::= itemlist COMMA expr",
 /* 175 */ "itemlist ::= expr",
 /* 176 */ "inscollist_opt ::=",
 /* 177 */ "inscollist_opt ::= LP RP",
 /* 178 */ "inscollist_opt ::= LP inscollist RP",
 /* 179 */ "inscollist ::= inscollist COMMA nm",
 /* 180 */ "inscollist ::= nm",
 /* 181 */ "expr ::= term",
 /* 182 */ "expr ::= LP expr RP",
 /* 183 */ "term ::= NULL",
 /* 184 */ "expr ::= ID",
 /* 185 */ "expr ::= JOIN_KW",
 /* 186 */ "expr ::= nm DOT nm",
 /* 187 */ "expr ::= nm DOT nm DOT nm",
 /* 188 */ "term ::= INTEGER|FLOAT|BLOB",
 /* 189 */ "term ::= STRING",
 /* 190 */ "expr ::= REGISTER",
 /* 191 */ "expr ::= VARIABLE",
 /* 192 */ "expr ::= VARIABLE1",
 /* 193 */ "expr ::= CAST LP expr AS typetoken RP",
 /* 194 */ "expr ::= ID LP distinct exprlist RP",
 /* 195 */ "expr ::= ID LP STAR RP",
 /* 196 */ "term ::= CTIME_KW",
 /* 197 */ "expr ::= expr AND expr",
 /* 198 */ "expr ::= expr OR expr",
 /* 199 */ "expr ::= expr LT|GT|GE|LE expr",
 /* 200 */ "expr ::= expr EQ|NE expr",
 /* 201 */ "expr ::= expr BITAND|BITOR|LSHIFT|RSHIFT expr",
 /* 202 */ "expr ::= expr PLUS|MINUS expr",
 /* 203 */ "expr ::= expr STAR|SLASH|REM expr",
 /* 204 */ "expr ::= expr CONCAT expr",
 /* 205 */ "likeop ::= LIKE_KW",
 /* 206 */ "likeop ::= NOT LIKE_KW",
 /* 207 */ "escape ::= ESCAPE expr",
 /* 208 */ "escape ::=",
 /* 209 */ "expr ::= expr likeop expr escape",
 /* 210 */ "expr ::= expr ISNULL|NOTNULL",
 /* 211 */ "expr ::= expr IS NULL",
 /* 212 */ "expr ::= expr NOT NULL",
 /* 213 */ "expr ::= expr IS NOT NULL",
 /* 214 */ "expr ::= NOT|BITNOT expr",
 /* 215 */ "expr ::= MINUS expr",
 /* 216 */ "expr ::= PLUS expr",
 /* 217 */ "between_op ::= BETWEEN",
 /* 218 */ "between_op ::= NOT BETWEEN",
 /* 219 */ "between_elem ::= INTEGER|STRING",
 /* 220 */ "expr ::= expr between_op between_elem AND between_elem",
 /* 221 */ "in_op ::= IN",
 /* 222 */ "in_op ::= NOT IN",
 /* 223 */ "expr ::= expr in_op LP exprlist RP",
 /* 224 */ "expr ::= LP select RP",
 /* 225 */ "expr ::= expr in_op LP select RP",
 /* 226 */ "expr ::= expr in_op nm dbnm",
 /* 227 */ "expr ::= EXISTS LP select RP",
 /* 228 */ "expr ::= CASE case_operand case_exprlist case_else END",
 /* 229 */ "case_exprlist ::= case_exprlist WHEN expr THEN expr",
 /* 230 */ "case_exprlist ::= WHEN expr THEN expr",
 /* 231 */ "case_else ::= ELSE expr",
 /* 232 */ "case_else ::=",
 /* 233 */ "case_operand ::= expr",
 /* 234 */ "case_operand ::=",
 /* 235 */ "exprlist ::= exprlist COMMA expritem",
 /* 236 */ "exprlist ::= expritem",
 /* 237 */ "expritem ::= expr",
 /* 238 */ "expritem ::=",
 /* 239 */ "idxlist_opt ::=",
 /* 240 */ "idxlist_opt ::= LP idxlist RP",
 /* 241 */ "idxlist ::= idxlist COMMA idxitem collate sortorder",
 /* 242 */ "idxlist ::= idxitem collate sortorder",
 /* 243 */ "idxitem ::= nm",
 /* 244 */ "plus_num ::= plus_opt number",
 /* 245 */ "minus_num ::= MINUS number",
 /* 246 */ "number ::= INTEGER|FLOAT",
 /* 247 */ "plus_opt ::= PLUS",
 /* 248 */ "plus_opt ::=",
 /* 249 */ "raisetype ::= ROLLBACK",
 /* 250 */ "raisetype ::= ABORT",
 /* 251 */ "raisetype ::= FAIL",
 /* 252 */ "cmd ::= SET variable_assignment_list",
 /* 253 */ "cmd ::= SET NAMES ids",
 /* 254 */ "cmd ::= SET CHARACTER SET ids",
 /* 255 */ "variable_assignment_list ::= variable_assignment_list COMMA scope_qualifier user_var_name EQ expr",
 /* 256 */ "variable_assignment_list ::= scope_qualifier user_var_name EQ expr",
 /* 257 */ "scope_qualifier ::= GLOBAL",
 /* 258 */ "scope_qualifier ::= LOCAL",
 /* 259 */ "scope_qualifier ::= SESSION",
 /* 260 */ "scope_qualifier ::= VARIABLE1 DOT",
 /* 261 */ "scope_qualifier ::=",
 /* 262 */ "user_var_name ::= ids",
 /* 263 */ "user_var_name ::= VARIABLE",
 /* 264 */ "cmd ::= show_databes",
 /* 265 */ "cmd ::= show_tables",
 /* 266 */ "cmd ::= show_table_status",
 /* 267 */ "cmd ::= show_variables",
 /* 268 */ "cmd ::= show_collation",
 /* 269 */ "show_databes ::= SHOW DATABASES|SCHEMAS show_statement_pattern",
 /* 270 */ "show_tables ::= SHOW full_keyword TABLES from_db show_statement_pattern",
 /* 271 */ "show_table_status ::= SHOW TABLE STATUS from_db show_statement_pattern",
 /* 272 */ "show_variables ::= SHOW scope_qualifier VARIABLES show_statement_pattern",
 /* 273 */ "show_collation ::= SHOW COLLATION show_statement_pattern",
 /* 274 */ "full_keyword ::= JOIN_KW",
 /* 275 */ "full_keyword ::=",
 /* 276 */ "show_statement_pattern ::= LIKE_KW STRING|ID",
 /* 277 */ "show_statement_pattern ::= where_opt",
 /* 278 */ "from_db ::=",
 /* 279 */ "from_db ::= FROM|IN nm",
};
#endif /* NDEBUG */

/*
** This function returns the symbolic name associated with a token
** value.
*/
const char *sqlite3ParserTokenName(int tokenType){
#ifndef NDEBUG
  if( tokenType>0 && tokenType<(sizeof(yyTokenName)/sizeof(yyTokenName[0])) ){
    return yyTokenName[tokenType];
  }else{
    return "Unknown";
  }
#else
  return "";
#endif
}

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to sqlite3Parser and sqlite3ParserFree.
*/
void *sqlite3ParserAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(YYCODETYPE yymajor, YYMINORTYPE *yypminor){
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    case 165:
    case 200:
    case 217:
#line 391 "parse.y"
{sqlite3SelectDelete((yypminor->yy387));}
#line 1183 "parse.c"
      break;
    case 180:
    case 181:
    case 205:
    case 207:
    case 215:
    case 221:
    case 230:
    case 232:
    case 234:
    case 236:
    case 237:
#line 669 "parse.y"
{sqlite3ExprDelete((yypminor->yy314));}
#line 1198 "parse.c"
      break;
    case 184:
    case 193:
    case 203:
    case 206:
    case 208:
    case 210:
    case 220:
    case 223:
    case 224:
    case 228:
    case 235:
#line 913 "parse.y"
{sqlite3ExprListDelete((yypminor->yy322));}
#line 1213 "parse.c"
      break;
    case 199:
    case 204:
    case 212:
    case 213:
#line 519 "parse.y"
{sqlite3SrcListDelete((yypminor->yy259));}
#line 1221 "parse.c"
      break;
    case 209:
#line 580 "parse.y"
{
  sqlite3ExprDelete((yypminor->yy292).pLimit);
  sqlite3ExprDelete((yypminor->yy292).pOffset);
}
#line 1229 "parse.c"
      break;
    case 216:
    case 219:
    case 226:
#line 536 "parse.y"
{sqlite3IdListDelete((yypminor->yy384));}
#line 1236 "parse.c"
      break;
    case 227:
#line 640 "parse.y"
{sqlite3ValuesListDelete((yypminor->yy287));}
#line 1241 "parse.c"
      break;
    case 241:
#line 1129 "parse.y"
{ sqlite3ExprListDelete((yypminor->yy322)); }
#line 1246 "parse.c"
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor( yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from sqlite3ParserAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void sqlite3ParserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
  (*freeProc)((void*)pParser);
}

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_MAX || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
#ifdef YYFALLBACK
    int iFallback;            /* Fallback token */
    if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
           && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
           yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
      }
#endif
      return yy_find_shift_action(pParser, iFallback);
    }
#endif
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  int iLookAhead            /* The look-ahead token */
){
  int i;
  /* int stateno = pParser->yystack[pParser->yyidx].stateno; */
 
  if( stateno>YY_REDUCE_MAX ||
      (i = yy_reduce_ofst[stateno])==YY_REDUCE_USE_DFLT ){
    return yy_default[stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer ot the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
  if( yypParser->yyidx>=YYSTACKDEPTH ){
     sqlite3ParserARG_FETCH;
     yypParser->yyidx--;
#ifndef NDEBUG
     if( yyTraceFILE ){
       fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
     }
#endif
     while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
     /* Here code is inserted which will execute if the parser
     ** stack every overflows */
#line 43 "parse.y"

  sqlite3ErrorMsg(pParse, "parser stack overflow");
#line 1398 "parse.c"
     sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument var */
     return;
  }
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = yyNewState;
  yytos->major = yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 149, 1 },
  { 150, 2 },
  { 150, 1 },
  { 152, 1 },
  { 152, 3 },
  { 151, 1 },
  { 151, 3 },
  { 154, 0 },
  { 154, 1 },
  { 154, 3 },
  { 153, 2 },
  { 153, 2 },
  { 155, 0 },
  { 155, 1 },
  { 153, 2 },
  { 153, 2 },
  { 153, 2 },
  { 156, 6 },
  { 159, 0 },
  { 159, 3 },
  { 158, 1 },
  { 158, 0 },
  { 157, 5 },
  { 157, 2 },
  { 162, 3 },
  { 162, 1 },
  { 164, 0 },
  { 164, 2 },
  { 164, 4 },
  { 164, 6 },
  { 164, 5 },
  { 167, 0 },
  { 167, 1 },
  { 166, 3 },
  { 168, 1 },
  { 171, 1 },
  { 172, 1 },
  { 160, 1 },
  { 160, 1 },
  { 160, 1 },
  { 169, 0 },
  { 169, 1 },
  { 173, 1 },
  { 173, 4 },
  { 173, 6 },
  { 174, 1 },
  { 174, 2 },
  { 175, 1 },
  { 175, 1 },
  { 170, 2 },
  { 170, 0 },
  { 178, 3 },
  { 178, 1 },
  { 178, 2 },
  { 178, 4 },
  { 178, 3 },
  { 178, 3 },
  { 178, 2 },
  { 179, 1 },
  { 179, 2 },
  { 179, 3 },
  { 179, 4 },
  { 179, 2 },
  { 179, 4 },
  { 179, 4 },
  { 179, 1 },
  { 179, 2 },
  { 187, 0 },
  { 187, 1 },
  { 185, 0 },
  { 185, 2 },
  { 188, 2 },
  { 188, 3 },
  { 188, 3 },
  { 188, 3 },
  { 189, 2 },
  { 189, 2 },
  { 189, 1 },
  { 189, 1 },
  { 186, 3 },
  { 186, 2 },
  { 190, 0 },
  { 190, 2 },
  { 190, 2 },
  { 163, 0 },
  { 163, 2 },
  { 191, 3 },
  { 191, 2 },
  { 191, 1 },
  { 192, 2 },
  { 192, 7 },
  { 192, 5 },
  { 192, 5 },
  { 192, 10 },
  { 194, 0 },
  { 194, 1 },
  { 182, 0 },
  { 182, 3 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 153, 4 },
  { 198, 2 },
  { 198, 0 },
  { 153, 1 },
  { 165, 1 },
  { 165, 3 },
  { 201, 1 },
  { 201, 2 },
  { 201, 1 },
  { 200, 9 },
  { 202, 1 },
  { 202, 1 },
  { 202, 0 },
  { 210, 2 },
  { 210, 0 },
  { 203, 3 },
  { 203, 2 },
  { 203, 4 },
  { 211, 2 },
  { 211, 1 },
  { 211, 0 },
  { 204, 0 },
  { 204, 2 },
  { 213, 2 },
  { 213, 0 },
  { 212, 6 },
  { 212, 7 },
  { 217, 1 },
  { 217, 1 },
  { 161, 0 },
  { 161, 2 },
  { 199, 2 },
  { 214, 1 },
  { 214, 2 },
  { 214, 3 },
  { 214, 4 },
  { 215, 2 },
  { 215, 0 },
  { 216, 4 },
  { 216, 0 },
  { 208, 0 },
  { 208, 3 },
  { 220, 5 },
  { 220, 3 },
  { 221, 1 },
  { 183, 1 },
  { 183, 1 },
  { 183, 0 },
  { 222, 0 },
  { 222, 2 },
  { 206, 0 },
  { 206, 3 },
  { 207, 0 },
  { 207, 2 },
  { 209, 0 },
  { 209, 2 },
  { 209, 4 },
  { 209, 4 },
  { 153, 5 },
  { 205, 0 },
  { 205, 2 },
  { 153, 6 },
  { 224, 5 },
  { 224, 3 },
  { 153, 6 },
  { 153, 6 },
  { 153, 5 },
  { 153, 5 },
  { 225, 1 },
  { 225, 1 },
  { 227, 5 },
  { 227, 3 },
  { 227, 2 },
  { 228, 3 },
  { 228, 1 },
  { 226, 0 },
  { 226, 2 },
  { 226, 3 },
  { 219, 3 },
  { 219, 1 },
  { 181, 1 },
  { 181, 3 },
  { 180, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 3 },
  { 181, 5 },
  { 180, 1 },
  { 180, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 6 },
  { 181, 5 },
  { 181, 4 },
  { 180, 1 },
  { 181, 3 },
  { 181, 3 },
  { 181, 3 },
  { 181, 3 },
  { 181, 3 },
  { 181, 3 },
  { 181, 3 },
  { 181, 3 },
  { 229, 1 },
  { 229, 2 },
  { 230, 2 },
  { 230, 0 },
  { 181, 4 },
  { 181, 2 },
  { 181, 3 },
  { 181, 3 },
  { 181, 4 },
  { 181, 2 },
  { 181, 2 },
  { 181, 2 },
  { 231, 1 },
  { 231, 2 },
  { 232, 1 },
  { 181, 5 },
  { 233, 1 },
  { 233, 2 },
  { 181, 5 },
  { 181, 3 },
  { 181, 5 },
  { 181, 4 },
  { 181, 4 },
  { 181, 5 },
  { 235, 5 },
  { 235, 4 },
  { 236, 2 },
  { 236, 0 },
  { 234, 1 },
  { 234, 0 },
  { 223, 3 },
  { 223, 1 },
  { 237, 1 },
  { 237, 0 },
  { 184, 0 },
  { 184, 3 },
  { 193, 5 },
  { 193, 3 },
  { 238, 1 },
  { 176, 2 },
  { 177, 2 },
  { 240, 1 },
  { 239, 1 },
  { 239, 0 },
  { 197, 1 },
  { 197, 1 },
  { 197, 1 },
  { 153, 2 },
  { 153, 3 },
  { 153, 4 },
  { 241, 6 },
  { 241, 4 },
  { 242, 1 },
  { 242, 1 },
  { 242, 1 },
  { 242, 2 },
  { 242, 0 },
  { 243, 1 },
  { 243, 1 },
  { 153, 1 },
  { 153, 1 },
  { 153, 1 },
  { 153, 1 },
  { 153, 1 },
  { 244, 3 },
  { 245, 5 },
  { 246, 5 },
  { 247, 4 },
  { 248, 3 },
  { 250, 1 },
  { 250, 0 },
  { 249, 2 },
  { 249, 1 },
  { 251, 0 },
  { 251, 2 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  sqlite3ParserARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<sizeof(yyRuleName)/sizeof(yyRuleName[0]) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

#ifndef NDEBUG
  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  */
  memset(&yygotominor, 0, sizeof(yygotominor));
#endif

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 3:
      case 4:
{ sqlite3FinishCoding(pParse); }
      break;
    case 7:
{ sqlite3BeginParse(pParse, 0); }
      break;
    case 8:
{ sqlite3BeginParse(pParse, 1); }
      break;
    case 9:
{ sqlite3BeginParse(pParse, 2); }
      break;
    case 10:
{sqlite3BeginTransaction(pParse, SQLTYPE_TRANSACTION_BEGIN);}
      break;
    case 11:
{sqlite3BeginTransaction(pParse, SQLTYPE_TRANSACTION_START);}
      break;
    case 14:
{sqlite3CommitTransaction(pParse);}
      break;
    case 15:
{sqlite3RollbackTransaction(pParse);}
      break;
    case 17:
{
   sqlite3StartTable(pParse, 0, 0, 0, 0, 0);
}
      break;
    case 18:
    case 21:
    case 67:
    case 81:
    case 83:
    case 94:
    case 103:
    case 112:
    case 113:
    case 217:
    case 221:
{yygotominor.yy4 = 0;}
      break;
    case 19:
    case 20:
    case 68:
    case 82:
    case 102:
    case 111:
    case 218:
    case 222:
{yygotominor.yy4 = 1;}
        break;
      case 22:
{
  //sqlite3EndTable(pParse,&X,&Y,0);
}
      break;
    case 23:
{
  sqlite3EndTable(pParse,0,0,0);
  //sqlite3SelectDelete(S);
  yy_destructor(165,&yymsp[0].minor);
}
      break;
    case 33:
{
  //A.z = X.z;
  //A.n = (pParse->sLastToken.z-X.z) + pParse->sLastToken.n;
}
      break;
    case 34:
{
  //sqlite3AddColumn(pParse,&X);
  //A = X;
}
      break;
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 246:
{yygotominor.yy378 = yymsp[0].minor.yy0;}
      break;
    case 41:
{ /*sqlite3AddColumnType(pParse,&X);*/ }
      break;
    case 42:
    case 45:
    case 119:
    case 120:
    case 131:
    case 150:
    case 243:
    case 244:
    case 245:
{yygotominor.yy378 = yymsp[0].minor.yy378;}
      break;
    case 43:
{
  yygotominor.yy378.z = yymsp[-3].minor.yy378.z;
  yygotominor.yy378.n = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] - yymsp[-3].minor.yy378.z;
}
      break;
    case 44:
{
  yygotominor.yy378.z = yymsp[-5].minor.yy378.z;
  yygotominor.yy378.n = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] - yymsp[-5].minor.yy378.z;
}
      break;
    case 46:
#line 253 "parse.y"
{yygotominor.yy378.z=yymsp[-1].minor.yy378.z; yygotominor.yy378.n=yymsp[0].minor.yy378.n+(yymsp[0].minor.yy378.z-yymsp[-1].minor.yy378.z);}
#line 1907 "parse.c"
        break;
      case 47:
#line 255 "parse.y"
{ yygotominor.yy4 = atoi((char*)yymsp[0].minor.yy378.z); }
#line 1912 "parse.c"
        break;
      case 48:
#line 256 "parse.y"
{ yygotominor.yy4 = -atoi((char*)yymsp[0].minor.yy378.z); }
#line 1917 "parse.c"
        break;
      case 53:
#line 265 "parse.y"
{ /*sqlite3AddDefaultValue(pParse,X);*/   yy_destructor(180,&yymsp[0].minor);
}
#line 1923 "parse.c"
        break;
      case 54:
#line 266 "parse.y"
{ /*sqlite3AddDefaultValue(pParse,X);*/ }
#line 1928 "parse.c"
        break;
      case 55:
#line 267 "parse.y"
{ /*sqlite3AddDefaultValue(pParse,X);*/  yy_destructor(180,&yymsp[0].minor);
}
#line 1934 "parse.c"
        break;
      case 56:
#line 268 "parse.y"
{
  //Expr *p = sqlite3Expr(TK_UMINUS, X, 0, 0);
  //sqlite3AddDefaultValue(pParse,p);
  yy_destructor(180,&yymsp[0].minor);
}
#line 1943 "parse.c"
        break;
      case 57:
#line 272 "parse.y"
{
  //Expr *p = sqlite3Expr(TK_STRING, 0, 0, &X);
  //sqlite3AddDefaultValue(pParse,p);
}
#line 1951 "parse.c"
        break;
      case 60:
#line 282 "parse.y"
{/*sqlite3AddNotNull(pParse, R);*/}
#line 1956 "parse.c"
        break;
      case 61:
#line 284 "parse.y"
{/*sqlite3AddPrimaryKey(pParse,0,R,I,Z);*/}
#line 1961 "parse.c"
        break;
      case 62:
#line 285 "parse.y"
{/*sqlite3CreateIndex(pParse,0,0,0,0,R,0,0,0,0);*/}
#line 1966 "parse.c"
        break;
      case 63:
#line 286 "parse.y"
{/*sqlite3AddCheckConstraint(pParse,X);*/  sqlite3ExprDelete(yymsp[-1].minor.yy314); }
#line 1971 "parse.c"
        break;
      case 64:
#line 288 "parse.y"
{/*sqlite3CreateForeignKey(pParse,0,&T,TA,R);*/ sqlite3ExprListDelete(yymsp[-1].minor.yy322); }
#line 1976 "parse.c"
        break;
      case 65:
#line 289 "parse.y"
{/*sqlite3DeferForeignKey(pParse,D);*/}
#line 1981 "parse.c"
        break;
      case 66:
#line 290 "parse.y"
{/*sqlite3AddCollateType(pParse, (char*)C.z, C.n);*/}
#line 1986 "parse.c"
        break;
      case 69:
#line 303 "parse.y"
{ yygotominor.yy4 = OE_Restrict * 0x010101; }
#line 1991 "parse.c"
        break;
      case 70:
#line 304 "parse.y"
{ yygotominor.yy4 = (yymsp[-1].minor.yy4 & yymsp[0].minor.yy215.mask) | yymsp[0].minor.yy215.value; }
#line 1996 "parse.c"
        break;
      case 71:
#line 306 "parse.y"
{ yygotominor.yy215.value = 0;     yygotominor.yy215.mask = 0x000000; }
#line 2001 "parse.c"
        break;
      case 72:
#line 307 "parse.y"
{ yygotominor.yy215.value = yymsp[0].minor.yy4;     yygotominor.yy215.mask = 0x0000ff; }
#line 2006 "parse.c"
        break;
      case 73:
#line 308 "parse.y"
{ yygotominor.yy215.value = yymsp[0].minor.yy4<<8;  yygotominor.yy215.mask = 0x00ff00; }
#line 2011 "parse.c"
        break;
      case 74:
#line 309 "parse.y"
{ yygotominor.yy215.value = yymsp[0].minor.yy4<<16; yygotominor.yy215.mask = 0xff0000; }
#line 2016 "parse.c"
        break;
      case 75:
#line 311 "parse.y"
{ yygotominor.yy4 = OE_SetNull; }
#line 2021 "parse.c"
        break;
      case 76:
#line 312 "parse.y"
{ yygotominor.yy4 = OE_SetDflt; }
#line 2026 "parse.c"
        break;
      case 77:
#line 313 "parse.y"
{ yygotominor.yy4 = OE_Cascade; }
#line 2031 "parse.c"
        break;
      case 78:
#line 314 "parse.y"
{ yygotominor.yy4 = OE_Restrict; }
#line 2036 "parse.c"
        break;
      case 79:
      case 80:
      case 95:
      case 97:
      case 98:
#line 316 "parse.y"
{yygotominor.yy4 = yymsp[0].minor.yy4;}
#line 2045 "parse.c"
        break;
      case 90:
#line 335 "parse.y"
{ sqlite3ExprListDelete(yymsp[-3].minor.yy322); }
#line 2050 "parse.c"
        break;
      case 91:
#line 337 "parse.y"
{/*sqlite3CreateIndex(pParse,0,0,0,yymsp[-2].minor.yy322,R,0,0,0,0);*/ sqlite3ExprListDelete(yymsp[-2].minor.yy322);}
#line 2055 "parse.c"
        break;
      case 92:
#line 338 "parse.y"
{/*sqlite3AddCheckConstraint(pParse,yymsp[-2].minor.yy314);*/ sqlite3ExprDelete(yymsp[-2].minor.yy314);}
#line 2060 "parse.c"
        break;
      case 93:
#line 340 "parse.y"
{ 
        sqlite3ExprListDelete(yymsp[-6].minor.yy322);
        sqlite3ExprListDelete(yymsp[-2].minor.yy322);
 }
#line 2068 "parse.c"
        break;
      case 96:
#line 355 "parse.y"
{yygotominor.yy4 = OE_Default;}
#line 2073 "parse.c"
        break;
      case 99:
#line 360 "parse.y"
{yygotominor.yy4 = OE_Ignore;}
#line 2078 "parse.c"
        break;
      case 100:
      case 170:
#line 361 "parse.y"
{yygotominor.yy4 = OE_Replace;}
#line 2084 "parse.c"
        break;
      case 101:
#line 365 "parse.y"
{
  sqlite3DropTable(pParse, yymsp[0].minor.yy259, 0, yymsp[-1].minor.yy4);
}
#line 2091 "parse.c"
        break;
      case 104:
#line 385 "parse.y"
{
  sqlite3Select(pParse, yymsp[0].minor.yy387, SRT_Callback, 0, 0, 0, 0, 0);
  // sqlite3SelectDelete(yymsp[0].minor.yy387);
}
#line 2099 "parse.c"
        break;
      case 105:
      case 128:
#line 395 "parse.y"
{yygotominor.yy387 = yymsp[0].minor.yy387;}
#line 2105 "parse.c"
        break;
      case 106:
#line 397 "parse.y"
{
  if( yymsp[0].minor.yy387 ){
    yymsp[0].minor.yy387->op = yymsp[-1].minor.yy4;
    yymsp[0].minor.yy387->pPrior = yymsp[-2].minor.yy387;
  }
  yygotominor.yy387 = yymsp[0].minor.yy387;
}
#line 2116 "parse.c"
        break;
      case 107:
      case 109:
#line 405 "parse.y"
{yygotominor.yy4 = yymsp[0].major;}
#line 2122 "parse.c"
        break;
      case 108:
#line 406 "parse.y"
{yygotominor.yy4 = TK_ALL;}
#line 2127 "parse.c"
        break;
      case 110:
#line 410 "parse.y"
{
  yygotominor.yy387 = sqlite3SelectNew(yymsp[-6].minor.yy322,yymsp[-5].minor.yy259,yymsp[-4].minor.yy314,yymsp[-3].minor.yy322,yymsp[-2].minor.yy314,yymsp[-1].minor.yy322,yymsp[-7].minor.yy4,yymsp[0].minor.yy292.pLimit,yymsp[0].minor.yy292.pOffset);
}
#line 2134 "parse.c"
        break;
      case 114:
      case 240:
#line 431 "parse.y"
{yygotominor.yy322 = yymsp[-1].minor.yy322;}
#line 2140 "parse.c"
        break;
      case 115:
      case 141:
      case 151:
      case 239:
#line 432 "parse.y"
{yygotominor.yy322 = 0;}
#line 2148 "parse.c"
        break;
      case 116:
#line 433 "parse.y"
{
   yygotominor.yy322 = sqlite3ExprListAppend(yymsp[-2].minor.yy322,yymsp[-1].minor.yy314,yymsp[0].minor.yy378.n?&yymsp[0].minor.yy378:0);
}
#line 2155 "parse.c"
        break;
      case 117:
#line 436 "parse.y"
{
  yygotominor.yy322 = sqlite3ExprListAppend(yymsp[-1].minor.yy322, sqlite3Expr(TK_ALL, 0, 0, 0), 0);
}
#line 2162 "parse.c"
        break;
      case 118:
#line 439 "parse.y"
{
  Expr *pRight = sqlite3Expr(TK_ALL, 0, 0, 0);
  Expr *pLeft = sqlite3Expr(TK_ID, 0, 0, &yymsp[-2].minor.yy378);
  yygotominor.yy322 = sqlite3ExprListAppend(yymsp[-3].minor.yy322, sqlite3Expr(TK_DOT, pLeft, pRight, 0), 0);
}
#line 2171 "parse.c"
        break;
      case 121:
#line 451 "parse.y"
{yygotominor.yy378.n = 0;}
#line 2176 "parse.c"
        break;
      case 122:
#line 463 "parse.y"
{yygotominor.yy259 = sqliteMalloc(sizeof(*yygotominor.yy259));}
#line 2181 "parse.c"
        break;
      case 123:
#line 464 "parse.y"
{yygotominor.yy259 = yymsp[0].minor.yy259;}
#line 2186 "parse.c"
        break;
      case 124:
#line 469 "parse.y"
{
   yygotominor.yy259 = yymsp[-1].minor.yy259;
   if( yygotominor.yy259 && yygotominor.yy259->nSrc>0 ) yygotominor.yy259->a[yygotominor.yy259->nSrc-1].jointype = yymsp[0].minor.yy4;
}
#line 2194 "parse.c"
        break;
      case 125:
#line 473 "parse.y"
{yygotominor.yy259 = 0;}
#line 2199 "parse.c"
        break;
      case 126:
#line 474 "parse.y"
{
  yygotominor.yy259 = sqlite3SrcListAppend(yymsp[-5].minor.yy259,&yymsp[-4].minor.yy378,&yymsp[-3].minor.yy378);
  if( yymsp[-2].minor.yy378.n ) sqlite3SrcListAddAlias(yygotominor.yy259,&yymsp[-2].minor.yy378);
  if( yymsp[-1].minor.yy314 ){
    if( yygotominor.yy259 && yygotominor.yy259->nSrc>1 ){ yygotominor.yy259->a[yygotominor.yy259->nSrc-2].pOn = yymsp[-1].minor.yy314; }
    else { sqlite3ExprDelete(yymsp[-1].minor.yy314); }
  }
  if( yymsp[0].minor.yy384 ){
    if( yygotominor.yy259 && yygotominor.yy259->nSrc>1 ){ yygotominor.yy259->a[yygotominor.yy259->nSrc-2].pUsing = yymsp[0].minor.yy384; }
    else { sqlite3IdListDelete(yymsp[0].minor.yy384); }
  }
}
#line 2215 "parse.c"
        break;
      case 127:
#line 488 "parse.y"
{
    yygotominor.yy259 = sqlite3SrcListAppend(yymsp[-6].minor.yy259,0,0);
    yygotominor.yy259->a[yygotominor.yy259->nSrc-1].pSelect = yymsp[-4].minor.yy387;
    if( yymsp[-2].minor.yy378.n ) sqlite3SrcListAddAlias(yygotominor.yy259,&yymsp[-2].minor.yy378);
    if( yymsp[-1].minor.yy314 ){
      if( yygotominor.yy259 && yygotominor.yy259->nSrc>1 ){ yygotominor.yy259->a[yygotominor.yy259->nSrc-2].pOn = yymsp[-1].minor.yy314; }
      else { sqlite3ExprDelete(yymsp[-1].minor.yy314); }
    }
    if( yymsp[0].minor.yy384 ){
      if( yygotominor.yy259 && yygotominor.yy259->nSrc>1 ){ yygotominor.yy259->a[yygotominor.yy259->nSrc-2].pUsing = yymsp[0].minor.yy384; }
      else { sqlite3IdListDelete(yymsp[0].minor.yy384); }
    }
  }
#line 2232 "parse.c"
        break;
      case 129:
#line 509 "parse.y"
{
     yygotominor.yy387 = sqlite3SelectNew(0,yymsp[0].minor.yy259,0,0,0,0,0,0,0);
  }
#line 2239 "parse.c"
        break;
      case 130:
#line 515 "parse.y"
{yygotominor.yy378.z=0; yygotominor.yy378.n=0;}
#line 2244 "parse.c"
        break;
      case 132:
#line 520 "parse.y"
{yygotominor.yy259 = sqlite3SrcListAppend(0,&yymsp[-1].minor.yy378,&yymsp[0].minor.yy378);}
#line 2249 "parse.c"
        break;
      case 133:
#line 524 "parse.y"
{ yygotominor.yy4 = JT_INNER; }
#line 2254 "parse.c"
        break;
      case 134:
#line 525 "parse.y"
{ yygotominor.yy4 = sqlite3JoinType(pParse,&yymsp[-1].minor.yy0,0,0); }
#line 2259 "parse.c"
        break;
      case 135:
#line 526 "parse.y"
{ yygotominor.yy4 = sqlite3JoinType(pParse,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy378,0); }
#line 2264 "parse.c"
        break;
      case 136:
#line 528 "parse.y"
{ yygotominor.yy4 = sqlite3JoinType(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy378,&yymsp[-1].minor.yy378); }
#line 2269 "parse.c"
        break;
      case 137:
      case 145:
      case 154:
      case 161:
      case 181:
      case 207:
      case 231:
      case 233:
      case 237:
#line 532 "parse.y"
{yygotominor.yy314 = yymsp[0].minor.yy314;}
#line 2282 "parse.c"
        break;
      case 138:
      case 153:
      case 160:
      case 208:
      case 232:
      case 234:
      case 238:
#line 533 "parse.y"
{yygotominor.yy314 = 0;}
#line 2293 "parse.c"
        break;
      case 139:
      case 178:
#line 537 "parse.y"
{yygotominor.yy384 = yymsp[-1].minor.yy384;}
#line 2299 "parse.c"
        break;
      case 140:
      case 176:
      case 177:
#line 538 "parse.y"
{yygotominor.yy384 = 0;}
#line 2306 "parse.c"
        break;
      case 142:
      case 152:
#line 549 "parse.y"
{yygotominor.yy322 = yymsp[0].minor.yy322;}
#line 2312 "parse.c"
        break;
      case 143:
#line 550 "parse.y"
{
  yygotominor.yy322 = sqlite3ExprListAppend(yymsp[-4].minor.yy322,yymsp[-2].minor.yy314,yymsp[-1].minor.yy378.n>0?&yymsp[-1].minor.yy378:0);
  if( yygotominor.yy322 ) yygotominor.yy322->a[yygotominor.yy322->nExpr-1].sortOrder = yymsp[0].minor.yy4;
}
#line 2320 "parse.c"
        break;
      case 144:
#line 554 "parse.y"
{
  yygotominor.yy322 = sqlite3ExprListAppend(0,yymsp[-2].minor.yy314,yymsp[-1].minor.yy378.n>0?&yymsp[-1].minor.yy378:0);
  if( yygotominor.yy322 && yygotominor.yy322->a ) yygotominor.yy322->a[0].sortOrder = yymsp[0].minor.yy4;
}
#line 2328 "parse.c"
        break;
      case 146:
      case 148:
#line 563 "parse.y"
{yygotominor.yy4 = SQLITE_SO_ASC;}
#line 2334 "parse.c"
        break;
      case 147:
#line 564 "parse.y"
{yygotominor.yy4 = SQLITE_SO_DESC;}
#line 2339 "parse.c"
        break;
      case 149:
#line 566 "parse.y"
{yygotominor.yy378.z = 0; yygotominor.yy378.n = 0;}
#line 2344 "parse.c"
        break;
      case 155:
#line 584 "parse.y"
{yygotominor.yy292.pLimit = 0; yygotominor.yy292.pOffset = 0;}
#line 2349 "parse.c"
        break;
      case 156:
#line 585 "parse.y"
{yygotominor.yy292.pLimit = yymsp[0].minor.yy314; yygotominor.yy292.pOffset = 0;}
#line 2354 "parse.c"
        break;
      case 157:
#line 587 "parse.y"
{yygotominor.yy292.pLimit = yymsp[-2].minor.yy314; yygotominor.yy292.pOffset = yymsp[0].minor.yy314;}
#line 2359 "parse.c"
        break;
      case 158:
#line 589 "parse.y"
{yygotominor.yy292.pOffset = yymsp[-2].minor.yy314; yygotominor.yy292.pLimit = yymsp[0].minor.yy314;}
#line 2364 "parse.c"
        break;
      case 159:
#line 593 "parse.y"
{sqlite3DeleteFrom(pParse,yymsp[-2].minor.yy259,yymsp[-1].minor.yy314, yymsp[0].minor.yy292.pLimit, yymsp[0].minor.yy292.pOffset);}
#line 2369 "parse.c"
        break;
      case 162:
#line 605 "parse.y"
{sqlite3Update(pParse,yymsp[-4].minor.yy259,yymsp[-2].minor.yy322,yymsp[-1].minor.yy314,OE_Default, yymsp[0].minor.yy292.pLimit, yymsp[0].minor.yy292.pOffset);}
#line 2374 "parse.c"
        break;
      case 163:
#line 611 "parse.y"
{yygotominor.yy322 = sqlite3ExprListAppend(yymsp[-4].minor.yy322,yymsp[0].minor.yy314,&yymsp[-2].minor.yy378);}
#line 2379 "parse.c"
        break;
      case 164:
#line 612 "parse.y"
{yygotominor.yy322 = sqlite3ExprListAppend(0,yymsp[0].minor.yy314,&yymsp[-2].minor.yy378);}
#line 2384 "parse.c"
        break;
      case 165:
#line 621 "parse.y"
{sqlite3Insert(pParse, yymsp[-3].minor.yy259, 0, yymsp[0].minor.yy287, 0, yymsp[-2].minor.yy384, yymsp[-5].minor.yy4);}
#line 2389 "parse.c"
        break;
      case 166:
#line 625 "parse.y"
{sqlite3Insert(pParse, yymsp[-3].minor.yy259, yymsp[0].minor.yy322, 0, 0, yymsp[-2].minor.yy384, yymsp[-5].minor.yy4);}
#line 2394 "parse.c"
        break;
      case 167:
#line 629 "parse.y"
{sqlite3Insert(pParse, yymsp[-3].minor.yy259, yymsp[0].minor.yy322, 0, 0, yymsp[-2].minor.yy384, yymsp[-4].minor.yy4);}
#line 2399 "parse.c"
        break;
      case 168:
#line 632 "parse.y"
{sqlite3Insert(pParse, yymsp[-2].minor.yy259, 0, 0, yymsp[0].minor.yy387, yymsp[-1].minor.yy384, yymsp[-4].minor.yy4);}
#line 2404 "parse.c"
        break;
      case 169:
#line 636 "parse.y"
{ yygotominor.yy4 = OE_Default; }
#line 2409 "parse.c"
        break;
      case 171:
#line 642 "parse.y"
{ yygotominor.yy287 = sqlite3ValuesListAppend(yymsp[-4].minor.yy287, yymsp[-1].minor.yy322);}
#line 2414 "parse.c"
        break;
      case 172:
#line 643 "parse.y"
{ yygotominor.yy287 = sqlite3ValuesListAppend(0, yymsp[-1].minor.yy322); }
#line 2419 "parse.c"
        break;
      case 173:
#line 644 "parse.y"
{ yygotominor.yy287 = 0; }
#line 2424 "parse.c"
        break;
      case 174:
      case 235:
#line 649 "parse.y"
{yygotominor.yy322 = sqlite3ExprListAppend(yymsp[-2].minor.yy322,yymsp[0].minor.yy314,0);}
#line 2430 "parse.c"
        break;
      case 175:
      case 236:
#line 650 "parse.y"
{yygotominor.yy322 = sqlite3ExprListAppend(0,yymsp[0].minor.yy314,0);}
#line 2436 "parse.c"
        break;
      case 179:
#line 660 "parse.y"
{yygotominor.yy384 = sqlite3IdListAppend(yymsp[-2].minor.yy384,&yymsp[0].minor.yy378);}
#line 2441 "parse.c"
        break;
      case 180:
#line 661 "parse.y"
{yygotominor.yy384 = sqlite3IdListAppend(0,&yymsp[0].minor.yy378);}
#line 2446 "parse.c"
        break;
      case 182:
#line 672 "parse.y"
{yygotominor.yy314 = yymsp[-1].minor.yy314; sqlite3ExprSpan(yygotominor.yy314,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0); }
#line 2451 "parse.c"
        break;
      case 183:
      case 188:
      case 189:
#line 673 "parse.y"
{yygotominor.yy314 = sqlite3Expr(yymsp[0].major, 0, 0, &yymsp[0].minor.yy0);}
#line 2458 "parse.c"
        break;
      case 184:
      case 185:
#line 674 "parse.y"
{yygotominor.yy314 = sqlite3Expr(TK_ID, 0, 0, &yymsp[0].minor.yy0);}
#line 2464 "parse.c"
        break;
      case 186:
#line 676 "parse.y"
{
  Expr *temp1 = sqlite3Expr(TK_ID, 0, 0, &yymsp[-2].minor.yy378);
  Expr *temp2 = sqlite3Expr(TK_ID, 0, 0, &yymsp[0].minor.yy378);
  yygotominor.yy314 = sqlite3Expr(TK_DOT, temp1, temp2, 0);
}
#line 2473 "parse.c"
        break;
      case 187:
#line 681 "parse.y"
{
  Expr *temp1 = sqlite3Expr(TK_ID, 0, 0, &yymsp[-4].minor.yy378);
  Expr *temp2 = sqlite3Expr(TK_ID, 0, 0, &yymsp[-2].minor.yy378);
  Expr *temp3 = sqlite3Expr(TK_ID, 0, 0, &yymsp[0].minor.yy378);
  Expr *temp4 = sqlite3Expr(TK_DOT, temp2, temp3, 0);
  yygotominor.yy314 = sqlite3Expr(TK_DOT, temp1, temp4, 0);
}
#line 2484 "parse.c"
        break;
      case 190:
#line 690 "parse.y"
{yygotominor.yy314 = sqlite3RegisterExpr(pParse, &yymsp[0].minor.yy0);}
#line 2489 "parse.c"
        break;
      case 191:
      case 192:
#line 691 "parse.y"
{
  Token *pToken = &yymsp[0].minor.yy0;
  Expr *pExpr = yygotominor.yy314 = sqlite3Expr(TK_VARIABLE, 0, 0, pToken);
  //sqlite3ExprAssignVarNumber(pParse, pExpr);
}
#line 2499 "parse.c"
        break;
      case 193:
#line 702 "parse.y"
{
  yygotominor.yy314 = sqlite3Expr(TK_CAST, yymsp[-3].minor.yy314, 0, &yymsp[-1].minor.yy378);
  sqlite3ExprSpan(yygotominor.yy314,&yymsp[-5].minor.yy0,&yymsp[0].minor.yy0);
}
#line 2507 "parse.c"
        break;
      case 194:
#line 707 "parse.y"
{
  yygotominor.yy314 = sqlite3ExprFunction(yymsp[-1].minor.yy322, &yymsp[-4].minor.yy0);
  sqlite3ExprSpan(yygotominor.yy314,&yymsp[-4].minor.yy0,&yymsp[0].minor.yy0);
  if( yymsp[-2].minor.yy4 && yygotominor.yy314 ){
    yygotominor.yy314->flags |= EP_Distinct;
  }
}
#line 2518 "parse.c"
        break;
      case 195:
#line 714 "parse.y"
{
  yygotominor.yy314 = sqlite3ExprFunction(0, &yymsp[-3].minor.yy0);
  sqlite3ExprSpan(yygotominor.yy314,&yymsp[-3].minor.yy0,&yymsp[0].minor.yy0);
}
#line 2526 "parse.c"
        break;
      case 196:
#line 718 "parse.y"
{
  /* The CURRENT_TIME, CURRENT_DATE, and CURRENT_TIMESTAMP values are
  ** treated as functions that return constants */
  yygotominor.yy314 = sqlite3ExprFunction(0,&yymsp[0].minor.yy0);
  if( yygotominor.yy314 ) yygotominor.yy314->op = TK_CONST_FUNC;  
}
#line 2536 "parse.c"
        break;
      case 197:
      case 198:
      case 199:
      case 200:
      case 201:
      case 202:
      case 203:
      case 204:
#line 724 "parse.y"
{yygotominor.yy314 = sqlite3Expr(yymsp[-1].major, yymsp[-2].minor.yy314, yymsp[0].minor.yy314, 0);}
#line 2548 "parse.c"
        break;
      case 205:
#line 734 "parse.y"
{yygotominor.yy342.eOperator = yymsp[0].minor.yy0; yygotominor.yy342.not = 0;}
#line 2553 "parse.c"
        break;
      case 206:
#line 735 "parse.y"
{yygotominor.yy342.eOperator = yymsp[0].minor.yy0; yygotominor.yy342.not = 1;}
#line 2558 "parse.c"
        break;
      case 209:
#line 740 "parse.y"
{
  ExprList *pList;
  pList = sqlite3ExprListAppend(0, yymsp[-3].minor.yy314, 0);
  pList = sqlite3ExprListAppend(pList, yymsp[-1].minor.yy314, 0);
  if( yymsp[0].minor.yy314 ){
    pList = sqlite3ExprListAppend(pList, yymsp[0].minor.yy314, 0);
  }
  //yygotominor.yy314 = sqlite3ExprFunction(pList, &yymsp[-2].minor.yy342.eOperator);
  yygotominor.yy314 = sqlite3ExprLikeOp(pList, &yymsp[-2].minor.yy342.eOperator);
  if( yymsp[-2].minor.yy342.not ) yygotominor.yy314 = sqlite3Expr(TK_NOT, yygotominor.yy314, 0, 0);
  sqlite3ExprSpan(yygotominor.yy314, &yymsp[-3].minor.yy314->span, &yymsp[-1].minor.yy314->span);
}
#line 2574 "parse.c"
        break;
      case 210:
#line 753 "parse.y"
{
  yygotominor.yy314 = sqlite3Expr(yymsp[0].major, yymsp[-1].minor.yy314, 0, 0);
  sqlite3ExprSpan(yygotominor.yy314,&yymsp[-1].minor.yy314->span,&yymsp[0].minor.yy0);
}
#line 2582 "parse.c"
        break;
      case 211:
#line 757 "parse.y"
{
  yygotominor.yy314 = sqlite3Expr(TK_ISNULL, yymsp[-2].minor.yy314, 0, 0);
  sqlite3ExprSpan(yygotominor.yy314,&yymsp[-2].minor.yy314->span,&yymsp[0].minor.yy0);
}
#line 2590 "parse.c"
        break;
      case 212:
#line 761 "parse.y"
{
  yygotominor.yy314 = sqlite3Expr(TK_NOTNULL, yymsp[-2].minor.yy314, 0, 0);
  sqlite3ExprSpan(yygotominor.yy314,&yymsp[-2].minor.yy314->span,&yymsp[0].minor.yy0);
}
#line 2598 "parse.c"
        break;
      case 213:
#line 765 "parse.y"
{
  yygotominor.yy314 = sqlite3Expr(TK_NOTNULL, yymsp[-3].minor.yy314, 0, 0);
  sqlite3ExprSpan(yygotominor.yy314,&yymsp[-3].minor.yy314->span,&yymsp[0].minor.yy0);
}
#line 2606 "parse.c"
        break;
      case 214:
#line 769 "parse.y"
{
  yygotominor.yy314 = sqlite3Expr(yymsp[-1].major, yymsp[0].minor.yy314, 0, 0);
  sqlite3ExprSpan(yygotominor.yy314,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy314->span);
}
#line 2614 "parse.c"
        break;
      case 215:
#line 773 "parse.y"
{
  yygotominor.yy314 = sqlite3Expr(TK_UMINUS, yymsp[0].minor.yy314, 0, 0);
  sqlite3ExprSpan(yygotominor.yy314,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy314->span);
}
#line 2622 "parse.c"
        break;
      case 216:
#line 777 "parse.y"
{
  yygotominor.yy314 = sqlite3Expr(TK_UPLUS, yymsp[0].minor.yy314, 0, 0);
  sqlite3ExprSpan(yygotominor.yy314,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy314->span);
}
#line 2630 "parse.c"
        break;
      case 219:
#line 788 "parse.y"
{ yygotominor.yy314 = sqlite3Expr(yymsp[0].major, 0, 0, &yymsp[0].minor.yy0); }
#line 2635 "parse.c"
        break;
      case 220:
#line 791 "parse.y"
{
  ExprList *pList = sqlite3ExprListAppend(0, yymsp[-2].minor.yy314, 0);
  pList = sqlite3ExprListAppend(pList, yymsp[0].minor.yy314, 0);
  yygotominor.yy314 = sqlite3Expr(TK_BETWEEN, yymsp[-4].minor.yy314, 0, 0);
  if( yygotominor.yy314 ){
    yygotominor.yy314->pList = pList;
  }else{
    sqlite3ExprListDelete(pList);
  } 
  if( yymsp[-3].minor.yy4 ) yygotominor.yy314 = sqlite3Expr(TK_NOT, yygotominor.yy314, 0, 0);
  sqlite3ExprSpan(yygotominor.yy314,&yymsp[-4].minor.yy314->span,&yymsp[0].minor.yy314->span);
}
#line 2651 "parse.c"
        break;
      case 223:
#line 807 "parse.y"
{
    yygotominor.yy314 = sqlite3Expr(TK_IN, yymsp[-4].minor.yy314, 0, 0);
    if( yygotominor.yy314 ){
      yygotominor.yy314->pList = yymsp[-1].minor.yy322;
    }else{
      sqlite3ExprListDelete(yymsp[-1].minor.yy322);
    }
    if( yymsp[-3].minor.yy4 ) yygotominor.yy314 = sqlite3Expr(TK_NOT, yygotominor.yy314, 0, 0);
    sqlite3ExprSpan(yygotominor.yy314,&yymsp[-4].minor.yy314->span,&yymsp[0].minor.yy0);
  }
#line 2665 "parse.c"
        break;
      case 224:
#line 817 "parse.y"
{
    yygotominor.yy314 = sqlite3Expr(TK_SELECT, 0, 0, 0);
    if( yygotominor.yy314 ){
      yygotominor.yy314->pSelect = yymsp[-1].minor.yy387;
    }else{
      sqlite3SelectDelete(yymsp[-1].minor.yy387);
    }
    sqlite3ExprSpan(yygotominor.yy314,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0);
  }
#line 2678 "parse.c"
        break;
      case 225:
#line 826 "parse.y"
{
    yygotominor.yy314 = sqlite3Expr(TK_IN, yymsp[-4].minor.yy314, 0, 0);
    if( yygotominor.yy314 ){
      yygotominor.yy314->pSelect = yymsp[-1].minor.yy387;
    }else{
      sqlite3SelectDelete(yymsp[-1].minor.yy387);
    }
    if( yymsp[-3].minor.yy4 ) yygotominor.yy314 = sqlite3Expr(TK_NOT, yygotominor.yy314, 0, 0);
    sqlite3ExprSpan(yygotominor.yy314,&yymsp[-4].minor.yy314->span,&yymsp[0].minor.yy0);
  }
#line 2692 "parse.c"
        break;
      case 226:
#line 836 "parse.y"
{
    SrcList *pSrc = sqlite3SrcListAppend(0,&yymsp[-1].minor.yy378,&yymsp[0].minor.yy378);
    yygotominor.yy314 = sqlite3Expr(TK_IN, yymsp[-3].minor.yy314, 0, 0);
    if( yygotominor.yy314 ){
      yygotominor.yy314->pSelect = sqlite3SelectNew(0,pSrc,0,0,0,0,0,0,0);
    }else{
      sqlite3SrcListDelete(pSrc);
    }
    if( yymsp[-2].minor.yy4 ) yygotominor.yy314 = sqlite3Expr(TK_NOT, yygotominor.yy314, 0, 0);
    sqlite3ExprSpan(yygotominor.yy314,&yymsp[-3].minor.yy314->span,yymsp[0].minor.yy378.z?&yymsp[0].minor.yy378:&yymsp[-1].minor.yy378);
  }
#line 2707 "parse.c"
        break;
      case 227:
#line 847 "parse.y"
{
    Expr *p = yygotominor.yy314 = sqlite3Expr(TK_EXISTS, 0, 0, 0);
    if( p ){
      p->pSelect = yymsp[-1].minor.yy387;
      sqlite3ExprSpan(p,&yymsp[-3].minor.yy0,&yymsp[0].minor.yy0);
    }else{
      sqlite3SelectDelete(yymsp[-1].minor.yy387);
    }
  }
#line 2720 "parse.c"
        break;
      case 228:
#line 859 "parse.y"
{
  yygotominor.yy314 = sqlite3Expr(TK_CASE, yymsp[-3].minor.yy314, yymsp[-1].minor.yy314, 0);
  if( yygotominor.yy314 ){
    yygotominor.yy314->pList = yymsp[-2].minor.yy322;
  }else{
    sqlite3ExprListDelete(yymsp[-2].minor.yy322);
  }
  sqlite3ExprSpan(yygotominor.yy314, &yymsp[-4].minor.yy0, &yymsp[0].minor.yy0);
}
#line 2733 "parse.c"
        break;
      case 229:
#line 870 "parse.y"
{
  yygotominor.yy322 = sqlite3ExprListAppend(yymsp[-4].minor.yy322, yymsp[-2].minor.yy314, 0);
  yygotominor.yy322 = sqlite3ExprListAppend(yygotominor.yy322, yymsp[0].minor.yy314, 0);
}
#line 2741 "parse.c"
        break;
      case 230:
#line 874 "parse.y"
{
  yygotominor.yy322 = sqlite3ExprListAppend(0, yymsp[-2].minor.yy314, 0);
  yygotominor.yy322 = sqlite3ExprListAppend(yygotominor.yy322, yymsp[0].minor.yy314, 0);
}
#line 2749 "parse.c"
        break;
      case 241:
#line 918 "parse.y"
{
  Expr *p = 0;
  if( yymsp[-1].minor.yy378.n>0 ){
    p = sqlite3Expr(TK_COLUMN, 0, 0, 0);
    if( p ) p->pColl = sqlite3LocateCollSeq(pParse, (char*)yymsp[-1].minor.yy378.z, yymsp[-1].minor.yy378.n);
  }
  yygotominor.yy322 = sqlite3ExprListAppend(yymsp[-4].minor.yy322, p, &yymsp[-2].minor.yy378);
  if( yygotominor.yy322 ) yygotominor.yy322->a[yygotominor.yy322->nExpr-1].sortOrder = yymsp[0].minor.yy4;
}
#line 2762 "parse.c"
        break;
      case 242:
#line 927 "parse.y"
{
  Expr *p = 0;
  if( yymsp[-1].minor.yy378.n>0 ){
    p = sqlite3Expr(TK_COLUMN, 0, 0, 0);
    if( p ) p->pColl = sqlite3LocateCollSeq(pParse, (char*)yymsp[-1].minor.yy378.z, yymsp[-1].minor.yy378.n);
  }
  yygotominor.yy322 = sqlite3ExprListAppend(0, p, &yymsp[-2].minor.yy378);
  if( yygotominor.yy322 ) yygotominor.yy322->a[yygotominor.yy322->nExpr-1].sortOrder = yymsp[0].minor.yy4;
}
#line 2775 "parse.c"
        break;
      case 249:
#line 1058 "parse.y"
{yygotominor.yy4 = OE_Rollback;}
#line 2780 "parse.c"
        break;
      case 250:
#line 1059 "parse.y"
{yygotominor.yy4 = OE_Abort;}
#line 2785 "parse.c"
        break;
      case 251:
#line 1060 "parse.y"
{yygotominor.yy4 = OE_Fail;}
#line 2790 "parse.c"
        break;
      case 252:
#line 1116 "parse.y"
{
    sqlite3SetStatement(pParse, yymsp[0].minor.yy322, 0, SQLTYPE_SET);
}
#line 2797 "parse.c"
        break;
      case 253:
#line 1120 "parse.y"
{
    sqlite3SetStatement(pParse, 0, &yymsp[0].minor.yy378, SQLTYPE_SET_NAMES);   
}
#line 2804 "parse.c"
        break;
      case 254:
#line 1124 "parse.y"
{
    sqlite3SetStatement(pParse, 0, &yymsp[0].minor.yy378, SQLTYPE_SET_CHARACTER_SET);
}
#line 2811 "parse.c"
        break;
      case 255:
#line 1130 "parse.y"
{
    yygotominor.yy322 = sqlite3ExprListAppend(yymsp[-5].minor.yy322, yymsp[0].minor.yy314, &yymsp[-2].minor.yy378);
}
#line 2818 "parse.c"
        break;
      case 256:
#line 1134 "parse.y"
{
    yygotominor.yy322 = sqlite3ExprListAppend(0, yymsp[0].minor.yy314, &yymsp[-2].minor.yy378);                         
}
#line 2825 "parse.c"
        break;
      case 260:
#line 1142 "parse.y"
{ sqlite3CheckSetScope(pParse, &yymsp[-1].minor.yy0); }
#line 2830 "parse.c"
        break;
      case 262:
#line 1146 "parse.y"
{ yygotominor.yy378 = yymsp[0].minor.yy378; }
#line 2835 "parse.c"
        break;
      case 263:
#line 1147 "parse.y"
{ yygotominor.yy378 = yymsp[0].minor.yy0; }
#line 2840 "parse.c"
        break;
      case 269:
#line 1156 "parse.y"
{
    sqlite3ShowStatement(pParse, SHOWTYPE_SHOW_DATABASES);
}
#line 2847 "parse.c"
        break;
      case 270:
#line 1160 "parse.y"
{
    sqlite3ShowStatement(pParse, SHOWTYPE_SHOW_TABLES);
}
#line 2854 "parse.c"
        break;
      case 271:
#line 1164 "parse.y"
{
    sqlite3ShowStatement(pParse, SHOWTYPE_SHOW_TABLE_STATUS);
}
#line 2861 "parse.c"
        break;
      case 272:
#line 1168 "parse.y"
{
    sqlite3ShowStatement(pParse, SHOWTYPE_SHOW_VARIABLES);
}
#line 2868 "parse.c"
        break;
      case 273:
#line 1172 "parse.y"
{
    sqlite3ShowStatement(pParse, SHOWTYPE_SHOW_COLLATION);              
}
#line 2875 "parse.c"
        break;
      case 277:
#line 1180 "parse.y"
{
    if (yymsp[0].minor.yy314) {
        sqlite3ExprDelete(yymsp[0].minor.yy314);
    }
}
#line 2884 "parse.c"
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = yyact;
      yymsp->major = yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else if( yyact == YYNSTATE + YYNRULE + 1 ){
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  sqlite3ParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  sqlite3ParserARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 34 "parse.y"

  if( pParse->zErrMsg==0 ){
    if( TOKEN.z[0] ){
      sqlite3ErrorMsg(pParse, "near \"%T\": syntax error", &TOKEN);
    }else{
      sqlite3ErrorMsg(pParse, "incomplete SQL statement");
    }
  }
#line 2951 "parse.c"
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  sqlite3ParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "sqlite3ParserAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void sqlite3Parser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  sqlite3ParserTOKENTYPE yyminor       /* The value for the token */
  sqlite3ParserARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
    /* if( yymajor==0 ) return; // not sure why this was here... */
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  sqlite3ParserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,yymajor);
    if( yyact<YYNSTATE ){
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      if( yyendofinput && yypParser->yyidx>=0 ){
        yymajor = 0;
      }else{
        yymajor = YYNOCODE;
      }
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else if( yyact == YY_ERROR_ACTION ){
      int yymx;
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_shift_action(yypParser,YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }else{
      yy_accept(yypParser);
      yymajor = YYNOCODE;
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}

int main(int argc, char *argv[])
{
  Parse parseObj;
  char *errMsg = 0;
  memset(&parseObj, 0, sizeof(parseObj));
  sqlite3RunParser(&parseObj, argv[1], &errMsg);  
  return 0;
}
