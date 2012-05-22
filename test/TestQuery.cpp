#include "tightdb.hpp"
#include <UnitTest++.h>
#include "group.hpp"

using namespace tightdb;

TIGHTDB_TABLE_2(TupleTableType,
                first,  Int,
                second, String)

TIGHTDB_TABLE_2(BoolTupleTable,
                first,  Int,
                second, Bool)


TEST(TestQueryFindAll_range1)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(4, "a");
    ttt.add(7, "a");
    ttt.add(10, "a");
    ttt.add(1, "a");
    ttt.add(4, "a");
    ttt.add(7, "a");
    ttt.add(10, "a");
    ttt.add(1, "a");
    ttt.add(4, "a");
    ttt.add(7, "a");
    ttt.add(10, "a");

    Query q1 = ttt.where().second.equal("a");
    TableView tv1 = q1.find_all(ttt, 4, 10);
    CHECK_EQUAL(6, tv1.size());
}

TEST(TestQueryFindAll_range_or)
{
    TupleTableType ttt;

    ttt.add(1, "b");
    ttt.add(2, "a"); //// match
    ttt.add(3, "b"); //
    ttt.add(1, "a"); //// match
    ttt.add(2, "b"); //// match
    ttt.add(3, "a");
    ttt.add(1, "b");
    ttt.add(2, "a"); //// match
    ttt.add(3, "b"); //

    Query q1 = ttt.where().group().first.greater(1).Or().second.equal("a").end_group().first.less(3);
    TableView tv1 = q1.find_all(ttt, 1, 8);
    CHECK_EQUAL(4, tv1.size());

    TableView tv2 = q1.find_all(ttt, 2, 8);
    CHECK_EQUAL(3, tv2.size());

    TableView tv3 = q1.find_all(ttt, 1, 7);
    CHECK_EQUAL(3, tv3.size());
}


TEST(TestQueryDelete)
{
    TupleTableType ttt;

    ttt.add(1, "X");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(4, "a");
    ttt.add(5, "X");
    ttt.add(6, "X");

    Query q = ttt.where().second.equal("X");
    size_t r = q.remove(ttt);

    CHECK_EQUAL(4, r);
    CHECK_EQUAL(2, ttt.size());
    CHECK_EQUAL(2, ttt[0].first);
    CHECK_EQUAL(4, ttt[1].first);
}



TEST(TestQuerySimple)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");

    Query q1 = ttt.where().first.equal(2);

    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(1, tv1.size());
    CHECK_EQUAL(1, tv1.get_source_ndx(0));
}


TEST(TestQuerySubtable)
{
    Group group;
    TableRef table = group.get_table("test");

    // Create specification with sub-table
    Spec& s = table->get_spec();
    s.add_column(COLUMN_TYPE_INT,    "first");
    s.add_column(COLUMN_TYPE_STRING, "second");
    Spec sub = s.add_subtable_column("third");
        sub.add_column(COLUMN_TYPE_INT,    "sub_first");
        sub.add_column(COLUMN_TYPE_STRING, "sub_second");
    table->update_from_spec();

    CHECK_EQUAL(3, table->get_column_count());

    // Main table
    table->insert_int(0, 0, 111);
    table->insert_string(1, 0, "this");
    table->insert_table(2, 0);
    table->insert_done();

    table->insert_int(0, 1, 222);
    table->insert_string(1, 1, "is");
    table->insert_table(2, 1);
    table->insert_done();

    table->insert_int(0, 2, 333);
    table->insert_string(1, 2, "a test");
    table->insert_table(2, 2);
    table->insert_done();

    table->insert_int(0, 3, 444);
    table->insert_string(1, 3, "of queries");
    table->insert_table(2, 3);
    table->insert_done();


    // Sub tables
    TableRef subtable = table->get_subtable(2, 0);
    subtable->insert_int(0, 0, 11);
    subtable->insert_string(1, 0, "a");
    subtable->insert_done();

    subtable = table->get_subtable(2, 1);
    subtable->insert_int(0, 0, 22);
    subtable->insert_string(1, 0, "b");
    subtable->insert_done();
    subtable->insert_int(0, 1, 33);
    subtable->insert_string(1, 1, "c");
    subtable->insert_done();

    subtable = table->get_subtable(2, 2);
    subtable->insert_int(0, 0, 44);
    subtable->insert_string(1, 0, "d");
    subtable->insert_done();

    subtable = table->get_subtable(2, 3);
    subtable->insert_int(0, 0, 55);
    subtable->insert_string(1, 0, "e");
    subtable->insert_done();


    Query *q1 = new Query;
    q1->greater(0, 200);
    q1->subtable(2);
    q1->less(0, 50);
    q1->parent();
    TableView t1 = q1->find_all(*table, 0, (size_t)-1);
    CHECK_EQUAL(2, t1.size());
    CHECK_EQUAL(1, t1.get_source_ndx(0));
    CHECK_EQUAL(2, t1.get_source_ndx(1));
    delete q1;


    Query *q2 = new Query;
    q2->subtable(2);
    q2->greater(0, 50);
    q2->Or();
    q2->less(0, 20);
    q2->parent();
    TableView t2 = q2->find_all(*table, 0, (size_t)-1);
    CHECK_EQUAL(2, t2.size());
    CHECK_EQUAL(0, t2.get_source_ndx(0));
    CHECK_EQUAL(3, t2.get_source_ndx(1));
    delete q2;


    Query *q3 = new Query;
    q3->subtable(2);
    q3->greater(0, 50);
    q3->Or();
    q3->less(0, 20);
    q3->parent();
    q3->less(0, 300);
    TableView t3 = q3->find_all(*table, 0, (size_t)-1);
    CHECK_EQUAL(1, t3.size());
    CHECK_EQUAL(0, t3.get_source_ndx(0));
    delete q3;


    Query *q4 = new Query;
    q4->equal(0, (int64_t)333);
    q4->Or();
    q4->subtable(2);
    q4->greater(0, 50);
    q4->Or();
    q4->less(0, 20);
    q4->parent();
    TableView t4 = q4->find_all(*table, 0, (size_t)-1);
    delete q4;


    CHECK_EQUAL(3, t4.size());
    CHECK_EQUAL(0, t4.get_source_ndx(0));
    CHECK_EQUAL(2, t4.get_source_ndx(1));
    CHECK_EQUAL(3, t4.get_source_ndx(2));
}




TEST(TestQuerySort1)
{
    TupleTableType ttt;

    ttt.add(1, "a"); // 0
    ttt.add(2, "a"); // 1
    ttt.add(3, "X"); // 2
    ttt.add(1, "a"); // 3
    ttt.add(2, "a"); // 4
    ttt.add(3, "X"); // 5
    ttt.add(9, "a"); // 6
    ttt.add(8, "a"); // 7
    ttt.add(7, "X"); // 8

    // tv.get_source_ndx()  = 0, 2, 3, 5, 6, 7, 8
    // Vals         = 1, 3, 1, 3, 9, 8, 7
    // result       = 3, 0, 5, 2, 8, 7, 6

    Query q = ttt.where().first.not_equal(2);
    TableView tv = q.find_all(ttt);
    tv.sort(0);

    CHECK(tv.size() == 7);
    CHECK(tv.get_int(0, 0) == 1);
    CHECK(tv.get_int(0, 1) == 1);
    CHECK(tv.get_int(0, 2) == 3);
    CHECK(tv.get_int(0, 3) == 3);
    CHECK(tv.get_int(0, 4) == 7);
    CHECK(tv.get_int(0, 5) == 8);
    CHECK(tv.get_int(0, 6) == 9);
}



TEST(TestQuerySort_QuickSort)
{
    // Triggers QuickSort because range > len
    TupleTableType ttt;

    for(size_t t = 0; t < 1000; t++)
        ttt.add(rand() % 1100, "a"); // 0

    Query q = ttt.where();
    TableView tv = q.find_all(ttt);
    tv.sort(0);

    CHECK(tv.size() == 1000);
    for(size_t t = 1; t < tv.size(); t++) {
        CHECK(tv.get_int(0, t - 1) <= tv.get_int(0, t - 1));
    }
}

TEST(TestQuerySort_CountSort)
{
    // Triggers CountSort because range <= len
    TupleTableType ttt;

    for(size_t t = 0; t < 1000; t++)
        ttt.add(rand() % 900, "a"); // 0

    Query q = ttt.where();
    TableView tv = q.find_all(ttt);
    tv.sort(0);

    CHECK(tv.size() == 1000);
    for(size_t t = 1; t < tv.size(); t++) {
        CHECK(tv.get_int(0, t - 1) <= tv.get_int(0, t - 1));
    }
}


TEST(TestQuerySort_Descending)
{
    TupleTableType ttt;

    for(size_t t = 0; t < 1000; t++)
        ttt.add(rand() % 1100, "a"); // 0

    Query q = ttt.where();
    TableView tv = q.find_all(ttt);
    tv.sort(0, false);

    CHECK(tv.size() == 1000);
    for(size_t t = 1; t < tv.size(); t++) {
        CHECK(tv.get_int(0, t - 1) >= tv.get_int(0, t - 1));
    }
}


TEST(TestQuerySort_Dates)
{
    Table table;
    table.add_column(COLUMN_TYPE_DATE, "first");

    table.insert_date(0, 0, 1000);
    table.insert_done();
    table.insert_date(0, 1, 3000);
    table.insert_done();
    table.insert_date(0, 2, 2000);
    table.insert_done();

    Query *q = new Query();
    TableView tv = q->find_all(table);
    delete q;
    CHECK(tv.size() == 3);
    CHECK(tv.get_source_ndx(0) == 0);
    CHECK(tv.get_source_ndx(1) == 1);
    CHECK(tv.get_source_ndx(2) == 2);

    tv.sort(0);

    CHECK(tv.size() == 3);
    CHECK(tv.get_date(0, 0) == 1000);
    CHECK(tv.get_date(0, 1) == 2000);
    CHECK(tv.get_date(0, 2) == 3000);
}


TEST(TestQuerySort_Bools)
{
    Table table;
    table.add_column(COLUMN_TYPE_BOOL, "first");

    table.insert_bool(0, 0, true);
    table.insert_done();
    table.insert_bool(0, 0, false);
    table.insert_done();
    table.insert_bool(0, 0, true);
    table.insert_done();

    Query *q = new Query();
    TableView tv = q->find_all(table);
    delete q;
    tv.sort(0);

    CHECK(tv.size() == 3);
    CHECK(tv.get_bool(0, 0) == false);
    CHECK(tv.get_bool(0, 1) == true);
    CHECK(tv.get_bool(0, 2) == true);
}



TEST(TestQueryThreads)
{
    TupleTableType ttt;

    // Spread query search hits in an odd way to test more edge cases
    // (thread job size is THREAD_CHUNK_SIZE = 10)
    for(int i = 0; i < 100; i++) {
        for(int j = 0; j < 10; j++) {
            ttt.add(5, "a");
            ttt.add(j, "b");
            ttt.add(6, "c");
            ttt.add(6, "a");
            ttt.add(6, "b");
            ttt.add(6, "c");
            ttt.add(6, "a");
        }
    }
    Query q1 = ttt.where().first.equal(2).second.equal("b");

    // Note, set THREAD_CHUNK_SIZE to 1.000.000 or more for performance
    //q1.SetThreads(5);
    TableView tv = q1.find_all(ttt);

    CHECK_EQUAL(100, tv.size());
    for(int i = 0; i < 100; i++) {
        CHECK_EQUAL(i*7*10 + 14 + 1, tv.get_source_ndx(i));
    }
}



TEST(TestQuerySimple2)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");

    Query q1 = ttt.where().first.equal(2);
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(3, tv1.size());
    CHECK_EQUAL(1, tv1.get_source_ndx(0));
    CHECK_EQUAL(4, tv1.get_source_ndx(1));
    CHECK_EQUAL(7, tv1.get_source_ndx(2));
}


TEST(TestQueryLimit)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a"); //
    ttt.add(3, "X");
    ttt.add(1, "a");
    ttt.add(2, "a"); //
    ttt.add(3, "X");
    ttt.add(1, "a");
    ttt.add(2, "a"); //
    ttt.add(3, "X");
    ttt.add(1, "a");
    ttt.add(2, "a"); //
    ttt.add(3, "X");
    ttt.add(1, "a");
    ttt.add(2, "a"); //
    ttt.add(3, "X");

    Query q1 = ttt.where().first.equal(2);

    TableView tv1 = q1.find_all(ttt, 0, (size_t)-1, 2);
    CHECK_EQUAL(2, tv1.size());
    CHECK_EQUAL(1, tv1.get_source_ndx(0));
    CHECK_EQUAL(4, tv1.get_source_ndx(1));

    TableView tv2 = q1.find_all(ttt, tv1.get_source_ndx(tv1.size() - 1) + 1, (size_t)-1, 2);
    CHECK_EQUAL(2, tv2.size());
    CHECK_EQUAL(7, tv2.get_source_ndx(0));
    CHECK_EQUAL(10, tv2.get_source_ndx(1));

    TableView tv3 = q1.find_all(ttt, tv2.get_source_ndx(tv2.size() - 1) + 1, (size_t)-1, 2);
    CHECK_EQUAL(1, tv3.size());
    CHECK_EQUAL(13, tv3.get_source_ndx(0));
}

TEST(TestQueryFindNext)
{
    TupleTableType ttt;
    
    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(4, "a");
    ttt.add(5, "a");
    ttt.add(6, "X");
    ttt.add(7, "X");
    
    Query q1 = ttt.where().second.equal("X").first.greater(4);
    
    const size_t res1 = q1.find_next(ttt);
    const size_t res2 = q1.find_next(ttt, res1);
    const size_t res3 = q1.find_next(ttt, res2);
    
    CHECK_EQUAL(5, res1);
    CHECK_EQUAL(6, res2);
    CHECK_EQUAL((size_t)-1, res3); // no more matches
}

TEST(TestQueryFindAll1)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(4, "a");
    ttt.add(5, "a");
    ttt.add(6, "X");
    ttt.add(7, "X");

    Query q1 = ttt.where().second.equal("a").first.greater(2).first.not_equal(4);
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(4, tv1.get_source_ndx(0));

    Query q2 = ttt.where().second.equal("X").first.greater(4);
    TableView tv2 = q2.find_all(ttt);
    CHECK_EQUAL(5, tv2.get_source_ndx(0));
    CHECK_EQUAL(6, tv2.get_source_ndx(1));

}

TEST(TestQueryFindAll2)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(4, "a");
    ttt.add(5, "a");
    ttt.add(11, "X");
    ttt.add(0, "X");

    Query q2 = ttt.where().second.not_equal("a").first.less(3);
    TableView tv2 = q2.find_all(ttt);
    CHECK_EQUAL(6, tv2.get_source_ndx(0));
}

TEST(TestQueryFindAllBetween)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(4, "a");
    ttt.add(5, "a");
    ttt.add(11, "X");
    ttt.add(3, "X");

    Query q2 = ttt.where().first.between(3, 5);
    TableView tv2 = q2.find_all(ttt);
    CHECK_EQUAL(2, tv2.get_source_ndx(0));
    CHECK_EQUAL(3, tv2.get_source_ndx(1));
    CHECK_EQUAL(4, tv2.get_source_ndx(2));
    CHECK_EQUAL(6, tv2.get_source_ndx(3));
}


TEST(TestQueryFindAll_Range)
{
    TupleTableType ttt;

    ttt.add(5, "a");
    ttt.add(5, "a");
    ttt.add(5, "a");

    Query q1 = ttt.where().second.equal("a").first.greater(2).first.not_equal(4);
    TableView tv1 = q1.find_all(ttt, 1, 2);
    CHECK_EQUAL(1, tv1.get_source_ndx(0));
}


TEST(TestQueryFindAll_Or)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(4, "a");
    ttt.add(5, "a");
    ttt.add(6, "a");
    ttt.add(7, "X");

    // first == 5 || second == X
    Query q1 = ttt.where().first.equal(5).Or().second.equal("X");
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(3, tv1.size());
    CHECK_EQUAL(2, tv1.get_source_ndx(0));
    CHECK_EQUAL(4, tv1.get_source_ndx(1));
    CHECK_EQUAL(6, tv1.get_source_ndx(2));
}


TEST(TestQueryFindAll_Parans1)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(3, "X");
    ttt.add(4, "a");
    ttt.add(5, "a");
    ttt.add(11, "X");

    // first > 3 && (second == X)
    Query q1 = ttt.where().first.greater(3).group().second.equal("X").end_group();
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(1, tv1.size());
    CHECK_EQUAL(6, tv1.get_source_ndx(0));
}


TEST(TestQueryFindAll_OrParan)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(4, "a");
    ttt.add(5, "a");
    ttt.add(6, "a");
    ttt.add(7, "X");
    ttt.add(2, "X");

    // (first == 5 || second == X && first > 2)
    Query q1 = ttt.where().group().first.equal(5).Or().second.equal("X").first.greater(2).end_group();
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(3, tv1.size());
    CHECK_EQUAL(2, tv1.get_source_ndx(0));
    CHECK_EQUAL(4, tv1.get_source_ndx(1));
    CHECK_EQUAL(6, tv1.get_source_ndx(2));
}


TEST(TestQueryFindAll_OrNested0)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(3, "X");
    ttt.add(4, "a");
    ttt.add(5, "a");
    ttt.add(11, "X");
    ttt.add(8, "Y");

    // first > 3 && (first == 5 || second == X)
    Query q1 = ttt.where().first.greater(3).group().first.equal(5).Or().second.equal("X").end_group();
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(2, tv1.size());
    CHECK_EQUAL(5, tv1.get_source_ndx(0));
    CHECK_EQUAL(6, tv1.get_source_ndx(1));
}

TEST(TestQueryFindAll_OrNested)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(3, "X");
    ttt.add(4, "a");
    ttt.add(5, "a");
    ttt.add(11, "X");
    ttt.add(8, "Y");

    // first > 3 && (first == 5 || (second == X || second == Y))
    Query q1 = ttt.where().first.greater(3).group().first.equal(5).Or().group().second.equal("X").Or().second.equal("Y").end_group().end_group();
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(5, tv1.get_source_ndx(0));
    CHECK_EQUAL(6, tv1.get_source_ndx(1));
    CHECK_EQUAL(7, tv1.get_source_ndx(2));
}

TEST(TestQueryFindAll_OrPHP)
{
    TupleTableType ttt;

    ttt.add(1, "Joe");
    ttt.add(2, "Sara");
    ttt.add(3, "Jim");

    // (second == Jim || second == Joe) && first = 1
    Query q1 = ttt.where().group().second.equal("Jim").Or().second.equal("Joe").end_group().first.equal(1);
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(0, tv1.get_source_ndx(0));
}

TEST(TestQueryFindAllOr)
{
    TupleTableType ttt;

    ttt.add(1, "Joe");
    ttt.add(2, "Sara");
    ttt.add(3, "Jim");

    // (second == Jim || second == Joe) && first = 1
    Query q1 = ttt.where().group().second.equal("Jim").Or().second.equal("Joe").end_group().first.equal(3);
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(2, tv1.get_source_ndx(0));
}



 

TEST(TestQueryFindAll_Parans2)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(3, "X");
    ttt.add(4, "a");
    ttt.add(5, "a");
    ttt.add(11, "X");

    // ()((first > 3()) && (()))
    Query q1 = ttt.where().group().end_group().group().group().first.greater(3).group().end_group().end_group().group().group().end_group().end_group().end_group();
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(3, tv1.size());
    CHECK_EQUAL(4, tv1.get_source_ndx(0));
    CHECK_EQUAL(5, tv1.get_source_ndx(1));
    CHECK_EQUAL(6, tv1.get_source_ndx(2));
}

TEST(TestQueryFindAll_Parans4)
{
    TupleTableType ttt;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");
    ttt.add(3, "X");
    ttt.add(4, "a");
    ttt.add(5, "a");
    ttt.add(11, "X");

    // ()
    Query q1 = ttt.where().group().end_group();
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(7, tv1.size());
}


TEST(TestQueryFindAll_Bool)
{
    BoolTupleTable btt;

    btt.add(1, true);
    btt.add(2, false);
    btt.add(3, true);
    btt.add(3, false);

    Query q1 = btt.where().second.equal(true);
    TableView tv1 = q1.find_all(btt);
    CHECK_EQUAL(0, tv1.get_source_ndx(0));
    CHECK_EQUAL(2, tv1.get_source_ndx(1));

    Query q2 = btt.where().second.equal(false);
    TableView tv2 = q2.find_all(btt);
    CHECK_EQUAL(1, tv2.get_source_ndx(0));
    CHECK_EQUAL(3, tv2.get_source_ndx(1));
}

TEST(TestQueryFindAll_Begins)
{
    TupleTableType ttt;

    ttt.add(0, "fo");
    ttt.add(0, "foo");
    ttt.add(0, "foobar");

    Query q1 = ttt.where().second.begins_with("foo");
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(2, tv1.size());
    CHECK_EQUAL(1, tv1.get_source_ndx(0));
    CHECK_EQUAL(2, tv1.get_source_ndx(1));
}

TEST(TestQueryFindAll_Ends)
{
    TupleTableType ttt;

    ttt.add(0, "barfo");
    ttt.add(0, "barfoo");
    ttt.add(0, "barfoobar");

    Query q1 = ttt.where().second.ends_with("foo");
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(1, tv1.size());
    CHECK_EQUAL(1, tv1.get_source_ndx(0));
}


TEST(TestQueryFindAll_Contains)
{
    TupleTableType ttt;

    ttt.add(0, "foo");
    ttt.add(0, "foobar");
    ttt.add(0, "barfoo");
    ttt.add(0, "barfoobaz");
    ttt.add(0, "fo");
    ttt.add(0, "fobar");
    ttt.add(0, "barfo");

    Query q1 = ttt.where().second.contains("foo");
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(4, tv1.size());
    CHECK_EQUAL(0, tv1.get_source_ndx(0));
    CHECK_EQUAL(1, tv1.get_source_ndx(1));
    CHECK_EQUAL(2, tv1.get_source_ndx(2));
    CHECK_EQUAL(3, tv1.get_source_ndx(3));
}

TEST(TestQueryEnums)
{
    TupleTableType table;

    for (size_t i = 0; i < 5; ++i) {
        table.add(1, "abd");
        table.add(2, "eftg");
        table.add(5, "hijkl");
        table.add(8, "mnopqr");
        table.add(9, "stuvxyz");
    }

    table.optimize();

    Query q1 = table.where().second.equal("eftg");
    TableView tv1 = q1.find_all(table);

    CHECK_EQUAL(5, tv1.size());
    CHECK_EQUAL(1, tv1.get_source_ndx(0));
    CHECK_EQUAL(6, tv1.get_source_ndx(1));
    CHECK_EQUAL(11, tv1.get_source_ndx(2));
    CHECK_EQUAL(16, tv1.get_source_ndx(3));
    CHECK_EQUAL(21, tv1.get_source_ndx(4));
}

#if (defined(_WIN32) || defined(__WIN32__) || defined(_WIN64))

#define uY  "\x0CE\x0AB"              // greek capital letter upsilon with dialytika (U+03AB)
#define uYd "\x0CE\x0A5\x0CC\x088"    // decomposed form (Y followed by two dots)
#define uy  "\x0CF\x08B"              // greek small letter upsilon with dialytika (U+03AB)
#define uyd "\x0cf\x085\x0CC\x088"    // decomposed form (Y followed by two dots)

TEST(TestQueryCaseSensitivity)
{
    TupleTableType ttt;

    ttt.add(1, "BLAAbaergroed");

    Query q1 = ttt.where().second.equal("blaabaerGROED", false);
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(1, tv1.size());
    CHECK_EQUAL(0, tv1.get_source_ndx(0));
}

TEST(TestQueryUnicode2)
{
    TupleTableType ttt;

    ttt.add(1, uY);
    ttt.add(1, uYd);
    ttt.add(1, uy);
    ttt.add(1, uyd);

    Query q1 = ttt.where().second.equal(uY, false);
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(2, tv1.size());
    CHECK_EQUAL(0, tv1.get_source_ndx(0));
    CHECK_EQUAL(2, tv1.get_source_ndx(1));

    Query q2 = ttt.where().second.equal(uYd, false);
    TableView tv2 = q2.find_all(ttt);
    CHECK_EQUAL(2, tv2.size());
    CHECK_EQUAL(1, tv2.get_source_ndx(0));
    CHECK_EQUAL(3, tv2.get_source_ndx(1));

    Query q3 = ttt.where().second.equal(uYd, true);
    TableView tv3 = q3.find_all(ttt);
    CHECK_EQUAL(1, tv3.size());
    CHECK_EQUAL(1, tv3.get_source_ndx(0));
}

#define uA  "\x0c3\x085"         // danish capital A with ring above (as in BLAABAERGROED)
#define uAd "\x041\x0cc\x08a"    // decomposed form (A (41) followed by ring)
#define ua  "\x0c3\x0a5"         // danish lower case a with ring above (as in blaabaergroed)
#define uad "\x061\x0cc\x08a"    // decomposed form (a (41) followed by ring)

TEST(TestQueryUnicode3)
{
    TupleTableType ttt;

    ttt.add(1, uA);
    ttt.add(1, uAd);
    ttt.add(1, ua);
    ttt.add(1, uad);

    Query q1 = ttt.where().second.equal(uA, false);
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(2, tv1.size());
    CHECK_EQUAL(0, tv1.get_source_ndx(0));
    CHECK_EQUAL(2, tv1.get_source_ndx(1));

    Query q2 = ttt.where().second.equal(ua, false);
    TableView tv2 = q2.find_all(ttt);
    CHECK_EQUAL(2, tv2.size());
    CHECK_EQUAL(0, tv2.get_source_ndx(0));
    CHECK_EQUAL(2, tv2.get_source_ndx(1));


    Query q3 = ttt.where().second.equal(uad, false);
    TableView tv3 = q3.find_all(ttt);
    CHECK_EQUAL(2, tv3.size());
    CHECK_EQUAL(1, tv3.get_source_ndx(0));
    CHECK_EQUAL(3, tv3.get_source_ndx(1));

    Query q4 = ttt.where().second.equal(uad, true);
    TableView tv4 = q4.find_all(ttt);
    CHECK_EQUAL(1, tv4.size());
    CHECK_EQUAL(3, tv4.get_source_ndx(0));
}


TEST(TestQueryFindAll_BeginsUNICODE)
{
    TupleTableType ttt;

    ttt.add(0, uad "fo");
    ttt.add(0, uad "foo");
    ttt.add(0, uad "foobar");

    Query q1 = ttt.where().second.begins_with(uad "foo");
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(2, tv1.size());
    CHECK_EQUAL(1, tv1.get_source_ndx(0));
    CHECK_EQUAL(2, tv1.get_source_ndx(1));
}


TEST(TestQueryFindAll_EndsUNICODE)
{
    TupleTableType ttt;

    ttt.add(0, "barfo");
    ttt.add(0, "barfoo" uad);
    ttt.add(0, "barfoobar");

    Query q1 = ttt.where().second.ends_with("foo" uad);
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(1, tv1.size());
    CHECK_EQUAL(1, tv1.get_source_ndx(0));

    Query q2 = ttt.where().second.ends_with("foo" uAd, false);
    TableView tv2 = q2.find_all(ttt);
    CHECK_EQUAL(1, tv2.size());
    CHECK_EQUAL(1, tv2.get_source_ndx(0));
}


TEST(TestQueryFindAll_ContainsUNICODE)
{
    TupleTableType ttt;

    ttt.add(0, uad "foo");
    ttt.add(0, uad "foobar");
    ttt.add(0, "bar" uad "foo");
    ttt.add(0, uad "bar" uad "foobaz");
    ttt.add(0, uad "fo");
    ttt.add(0, uad "fobar");
    ttt.add(0, uad "barfo");

    Query q1 = ttt.where().second.contains(uad "foo");
    TableView tv1 = q1.find_all(ttt);
    CHECK_EQUAL(4, tv1.size());
    CHECK_EQUAL(0, tv1.get_source_ndx(0));
    CHECK_EQUAL(1, tv1.get_source_ndx(1));
    CHECK_EQUAL(2, tv1.get_source_ndx(2));
    CHECK_EQUAL(3, tv1.get_source_ndx(3));

    Query q2 = ttt.where().second.contains(uAd "foo", false);
    TableView tv2 = q1.find_all(ttt);
    CHECK_EQUAL(4, tv2.size());
    CHECK_EQUAL(0, tv2.get_source_ndx(0));
    CHECK_EQUAL(1, tv2.get_source_ndx(1));
    CHECK_EQUAL(2, tv2.get_source_ndx(2));
    CHECK_EQUAL(3, tv2.get_source_ndx(3));
}

#endif

TEST(TestQuerySyntaxCheck)
{
    TupleTableType ttt;
    std::string s;

    ttt.add(1, "a");
    ttt.add(2, "a");
    ttt.add(3, "X");

    Query q1 = ttt.where().first.equal(2).end_group();
    s = q1.Verify();
    CHECK(s != "");

    Query q2 = ttt.where().group().group().first.equal(2).end_group();
    s = q2.Verify();
    CHECK(s != "");

    Query q3 = ttt.where().first.equal(2).Or();
    s = q3.Verify();
    CHECK(s != "");

    Query q4 = ttt.where().Or().first.equal(2);
    s = q4.Verify();
    CHECK(s != "");

    Query q5 = ttt.where().first.equal(2);
    s = q5.Verify();
    CHECK(s == "");

    Query q6 = ttt.where().group().first.equal(2);
    s = q6.Verify();
    CHECK(s != "");

    Query q7 = ttt.where().second.equal("\xa0", false);
    s = q7.Verify();
    CHECK(s != "");
}
