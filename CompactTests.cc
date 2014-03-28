#include "gtest/gtest.h"
#include "internal.h"
#include "libcouchstore/couch_db.h"
#include "libcouchstore/couch_common.h"
#include "libcouchstore/couch_index.h"
#include "libcouchstore/error.h"
#include <cstring>
#include <iostream>
#include <libcouchbase/couchbase.h>
#include "dataclient.c"

extern char *getvaluebuf;
extern long *getvaluesize;
char **testargv;
char testargc;

class CompactionTest : public testing::Test {

protected:

   virtual void SetUp(){
   memset(&create_options, 0, sizeof(create_options));
   if (testargc > 0) {
        create_options.v.v0.host = testargv[1];
    }
    if (testargc > 1) {
        create_options.v.v0.user = testargv[2];
        create_options.v.v0.bucket = testargv[2];
    }
    if (testargc > 2) {
        create_options.v.v0.passwd = testargv[3];
    }
   err = lcb_create(&instance, &create_options);
   assert(err==LCB_SUCCESS);
   (void)lcb_set_error_callback(instance, error_callback);
   err = lcb_connect(instance);
   assert(err==LCB_SUCCESS);
   (void)lcb_set_get_callback(instance, get_callback);
   (void)lcb_set_store_callback(instance, store_callback);
      
       
   }


   virtual void TearDown(){
   lcb_destroy(instance);
   }
   void sendHello(){
   
   lcb_wait(instance);
   {
   err = lcb_hello(instance, NULL);
   assert(err==LCB_SUCCESS);
   }
   
   }
    
   void sendcompact(uint16_t vbid, uint64_t purge_before_ts, uint64_t purge_before_seq, uint8_t drop_deletes){
   
   lcb_wait(instance);
   {
   err = lcb_compact(instance, NULL,vbid,purge_before_ts,purge_before_seq,drop_deletes);
   assert(err==LCB_SUCCESS);
   }
   }
   
   std::string exec(char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
    	if(fgets(buffer, 128, pipe) != NULL)
    		result += buffer;
    }
    pclose(pipe);
    return result;
   }
 
   void compareDocs(const lcb_store_cmd_t *commands){
   EXPECT_EQ(*getvaluedtype,commands->v.v0.datatype);
   EXPECT_EQ(*getvaluesize,commands->v.v0.nbytes);
   for (long i=0; i<*getvaluesize;i++){
       EXPECT_EQ(*(getvaluebuf+i),*((char*)commands->v.v0.bytes+i)); 
       } 
   }
   
  void DatatypeTester(const lcb_store_cmd_t *commands) {
  lcb_wait(instance);
   {
   err = lcb_store(instance, NULL, 1, &commands);
   assert(err==LCB_SUCCESS);
   }
  lcb_wait(instance);
  callget(&instance, commands->v.v0.key, commands->v.v0.nkey);   
  lcb_wait(instance);
  fprintf(stderr, "\nInside DatatypeTester\n"); 
  compareDocs(commands);
  }
  lcb_uint32_t tmo;
  const lcb_store_cmd_t *commands[1];
  lcb_error_t err;
  lcb_t instance;
  struct lcb_create_st create_options;
};
/*
TEST_F(CompactionTest, SizeReductionTest) {
  couchstore_error_t error; 
  Db* db; 
  DbInfo* dbinfo;
  db = (Db*)malloc(sizeof(Db));
  dbinfo = (DbInfo*)malloc(sizeof(DbInfo));
  uint16_t vbid = 14;
  uint64_t purge_before_ts = 0;  
  uint64_t purge_before_seq =0;
  uint8_t drop_deletes = 0;
  uint64_t fsize_b4compact =0;
  uint64_t fsize_a4trcompact =0;
  const char *filename = "/Users/venu/couchbasemaster/ns_server/data/n_0/data/default/14.couch.1"; 
  error = couchstore_open_db(filename, COUCHSTORE_OPEN_FLAG_RDONLY, &db);
  fprintf(stderr, "\nopendb error %d \n",error); 
  error = couchstore_db_info(db, dbinfo);
  fprintf(stderr, "\ndbinfo error %d \n",error); 
  fprintf(stderr, "\nDocinfo file size used  %llu \n", dbinfo->file_size); 
  fsize_b4compact = dbinfo->file_size;
  couchstore_close_db(db);
  sendcompact(vbid, purge_before_ts, purge_before_seq, drop_deletes);  
  lcb_wait(instance);
  filename = "/Users/venu/couchbasemaster/ns_server/data/n_0/data/default/14.couch.2"; 
  error = couchstore_open_db(filename, COUCHSTORE_OPEN_FLAG_RDONLY, &db);
  fprintf(stderr, "\nopendb error %d \n",error); 
  error = couchstore_db_info(db, dbinfo);
  fprintf(stderr, "\ndbinfo error %d \n",error); 
  fprintf(stderr, "\nDocinfo filename %s \n", dbinfo->filename); 
  fprintf(stderr, "\nDocinfo doc count  %llu \n", dbinfo->doc_count); 
  fprintf(stderr, "\nDocinfo file size used  %llu \n", dbinfo->file_size); 
  fprintf(stderr, "\nDocinfo deleted count  %llu \n", dbinfo->deleted_count); 
  fsize_a4trcompact = dbinfo->file_size;
  EXPECT_LT(fsize_a4trcompact,fsize_b4compact);
}*/

TEST_F(CompactionTest, DropDeletesTest) {
  
  couchstore_error_t error; 
  Db* db1; 
  Db* db2; 
  DbInfo* dbinfo1;
  DbInfo* dbinfo2;
  DocInfo* docinfo1;
  DocInfo* docinfo2;
  
  db1 = (Db*)malloc(sizeof(Db));
  db2 = (Db*)malloc(sizeof(Db));
  dbinfo1 = (DbInfo*)malloc(sizeof(DbInfo));
  dbinfo2 = (DbInfo*)malloc(sizeof(DbInfo));
  docinfo1 = (DocInfo*)malloc(sizeof(DocInfo));
  docinfo2 = (DocInfo*)malloc(sizeof(DocInfo));
  
  const char* key = "fooaaa"; 
  uint16_t vbid = 14;
  uint64_t purge_before_ts = 0;  
  uint64_t purge_before_seq =0;
  uint8_t drop_deletes = 1;
  uint64_t fsize_b4compact =0;
  uint64_t fsize_a4trcompact =0;
  const char *filename = "/Users/venu/Library/Application Support/Couchbase/var/lib/couchdb/default/14.couch.1"; 

  error = couchstore_open_db(filename, COUCHSTORE_OPEN_FLAG_RDONLY, &db1);
  fprintf(stderr, "\nopendb error %d \n",error); 
  error = couchstore_docinfo_by_id(db1,(void*)key,6,&docinfo1);
  fprintf(stderr, "\ndocinfo by id error %d \n",error); 
  fprintf(stderr, "\ndocinfo deleted %d \n",docinfo1->deleted); 
  fprintf(stderr, "\ndocinfo doc size %lu \n",docinfo1->size); 
  EXPECT_EQ(0,error);
  error = couchstore_db_info(db1, dbinfo1);
  fprintf(stderr, "\ndbinfo error %d \n",error); 
  fprintf(stderr, "\nDocinfo file size used  %llu \n", dbinfo1->file_size); 
  fsize_b4compact = dbinfo1->file_size;
  couchstore_close_db(db1);
  sendcompact(vbid, purge_before_ts, purge_before_seq, drop_deletes);  
  lcb_wait(instance);
  filename = "/Users/venu/Library/Application Support/Couchbase/var/lib/couchdb/default/14.couch.2"; 
  error = couchstore_open_db(filename, COUCHSTORE_OPEN_FLAG_RDONLY, &db2);
  fprintf(stderr, "\nopendb error %d \n",error); 
  error = couchstore_docinfo_by_id(db2, (void*)key,6,&docinfo2);
  fprintf(stderr, "\ndocinfo by id error %d \n",error); 
  fprintf(stderr, "\ndocinfo deleted %d \n",docinfo2->deleted); 
  fprintf(stderr, "\ndocinfo doc size %lu \n",docinfo2->size); 
  EXPECT_EQ(-5,error);
  error = couchstore_db_info(db2, dbinfo2);
  fprintf(stderr, "\ndbinfo error %d \n",error); 
  fprintf(stderr, "\nDocinfo filename %s \n", dbinfo2->filename); 
  fprintf(stderr, "\nDocinfo doc count  %llu \n", dbinfo2->doc_count); 
  fprintf(stderr, "\nDocinfo file size used  %llu \n", dbinfo2->file_size); 
  fprintf(stderr, "\nDocinfo deleted count  %llu \n", dbinfo2->deleted_count); 
  fsize_a4trcompact = dbinfo2->file_size;
  EXPECT_LT(fsize_a4trcompact,fsize_b4compact);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  testargv = argv;
  testargc = argc;
  return RUN_ALL_TESTS();
}
