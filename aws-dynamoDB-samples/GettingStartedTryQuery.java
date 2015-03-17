// SAMPLE CODE
// http://docs.aws.amazon.com/amazondynamodb/latest/developerguide/dynamodb-dg.pdf
//
package com.amazonaws.codesamples;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Iterator;
import com.amazonaws.auth.profile.ProfileCredentialsProvider;
import com.amazonaws.services.dynamodbv2.AmazonDynamoDBClient;
import com.amazonaws.services.dynamodbv2.document.DynamoDB;
import com.amazonaws.services.dynamodbv2.document.Item;
import com.amazonaws.services.dynamodbv2.document.ItemCollection;
import com.amazonaws.services.dynamodbv2.document.QueryOutcome;
import com.amazonaws.services.dynamodbv2.document.RangeKeyCondition;
import com.amazonaws.services.dynamodbv2.document.Table;
import com.amazonaws.services.dynamodbv2.document.spec.QuerySpec;
public class GettingStartedTryQuery {
static DynamoDB dynamoDB = new DynamoDB(new AmazonDynamoDBClient(
new ProfileCredentialsProvider()));
static SimpleDateFormat dateFormatter = new SimpleDateFormat(
"yyyy-MM-dd'T'HH:mm:ss.SSS'Z'");
public static void main(String[] args) throws Exception {
try {
String forumName = "Amazon DynamoDB";
String threadSubject = "DynamoDB Thread 1";
// Get an item.
getBook(101, "ProductCatalog");
// Query replies posted in the past 15 days for a forum thread.
findRepliesInLast15DaysWithConfig("Reply", forumName, threadSubject);
} catch (Exception e) {
System.err.println(e.getMessage());
}
}
private static void getBook(int id, String tableName) {
Table table = dynamoDB.getTable(tableName);
Item item = table.getItem("Id", // attribute name
id, // attribute value
"Id, ISBN, Title, Authors", // projection expression
null); // name map - don't need this
System.out.println("GetItem: printing results...");
System.out.println(item.toJSONPretty());
}
 private static void findRepliesInLast15DaysWithConfig(
String tableName, String forumName, String threadSubject) {
String replyId = forumName + "#" + threadSubject;
long twoWeeksAgoMilli = (new Date()).getTime()
- (15L * 24L * 60L * 60L * 1000L);
Date twoWeeksAgo = new Date();
twoWeeksAgo.setTime(twoWeeksAgoMilli);
SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-
dd'T'HH:mm:ss.SSS'Z'");
String twoWeeksAgoStr = df.format(twoWeeksAgo);
Table table = dynamoDB.getTable(tableName);
QuerySpec querySpec = new QuerySpec()
.withHashKey("Id", replyId)
.withRangeKeyCondition(
new RangeKeyCondition("ReplyDateTime")
.gt(twoWeeksAgoStr))
.withProjectionExpression("Message, ReplyDateTime, PostedBy");
ItemCollection<QueryOutcome> items = table.query(querySpec);
Iterator<Item> iterator = items.iterator();
System.out.println("Query: printing results...");
while (iterator.hasNext()) {
System.out.println(iterator.next().toJSONPretty());
}
}
}
