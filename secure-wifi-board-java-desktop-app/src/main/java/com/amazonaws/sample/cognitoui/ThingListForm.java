package com.amazonaws.sample.cognitoui;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import com.amazonaws.AmazonServiceException;
import com.amazonaws.services.dynamodbv2.AmazonDynamoDB;
import com.amazonaws.services.dynamodbv2.AmazonDynamoDBClientBuilder;
import com.amazonaws.services.dynamodbv2.document.DynamoDB;
import com.amazonaws.services.dynamodbv2.document.Item;
import com.amazonaws.services.dynamodbv2.document.ItemCollection;
import com.amazonaws.services.dynamodbv2.document.ScanOutcome;
import com.amazonaws.services.dynamodbv2.document.Table;
import com.amazonaws.services.dynamodbv2.model.AttributeAction;
import com.amazonaws.services.dynamodbv2.model.AttributeValue;
import com.amazonaws.services.dynamodbv2.model.AttributeValueUpdate;
import com.amazonaws.services.dynamodbv2.model.ResourceNotFoundException;
import com.amazonaws.services.dynamodbv2.document.DeleteItemOutcome;
import com.amazonaws.services.cognitoidentity.model.*;
import com.amazonaws.services.cognitoidentity.model.Credentials;
import com.amazonaws.auth.AWSStaticCredentialsProvider;
import com.amazonaws.auth.BasicSessionCredentials;

import javafx.scene.control.*;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.TextArea;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.stage.Modality;
import javafx.stage.Stage;

class ThingListForm {

	 private static Credentials credentials;
	 static String accessKey;
	 static String secretKey;
	 static String sessionToken;
	 static final int COGNITO_UUID_LEN = 36;

     static void display(String title, String message) {
    	 
    	
        GridPane grid = new GridPane();
        grid.setAlignment(Pos.CENTER);
        grid.setHgap(10);
        grid.setVgap(10);
        grid.setPadding(new Insets(25, 25, 25, 25));
		

        Scene scene = new Scene(grid, 400, 500);

        Stage window = new Stage();
        window.initModality(Modality.APPLICATION_MODAL);
        window.setTitle(title);
        window.setMinWidth(250);
        TextArea scenetitle = new TextArea();

        scenetitle.setText(message);
        scenetitle.setFont(Font.font("Tahoma", FontWeight.NORMAL, 12));
        //grid.add(scenetitle, 0, 0, 2, 1);
        
		Label register_message = new Label("");
		Button registerButton = new Button("Register Board");
		Button clearThingButton = new Button("Clear Thing");
		registerButton.setMaxWidth(190);
		clearThingButton.setMaxWidth(190);
		VBox regBtn = new VBox(20);
        regBtn.setAlignment(Pos.BOTTOM_RIGHT);
        regBtn.getChildren().addAll(scenetitle, clearThingButton, registerButton, register_message);
        regBtn.setMaxWidth(360);
        grid.add(regBtn, 0, 0, 2, 1);
		regBtn.setAlignment(Pos.CENTER);

		

		/*
		HBox clBtn = new HBox(10);
		clBtn.setAlignment(Pos.BOTTOM_RIGHT);
        clBtn.getChildren().add(clearThingButton);
        //clBtn.setMaxWidth(190);
		clBtn.setPrefWidth(190);
        grid.add(clBtn, 1, 5);
		*/

		String[] lines = message.split(System.getProperty("line.separator"));
		String[] val = lines[0].split("=");
		String temp_uuid = val[1];

		String cognito_uuid = temp_uuid.substring(0, COGNITO_UUID_LEN);
		System.out.println("debug: cognito_uuid=" +cognito_uuid);
        registerButton.setOnAction(e -> {
			boolean found = false;
     
        	try{
				
				found = false;
				System.out.println("wait...");
				register_message.setText("wait..");

				String OS = System.getProperty("os.name").toLowerCase();
				Process p;
				if (OS.indexOf("mac") >= 0)
            		p = Runtime.getRuntime().exec("python3 kit_get_thing_id.py");
				else 
					p = Runtime.getRuntime().exec("python kit_get_thing_id.py");
            	 
            	BufferedReader in = new BufferedReader(new InputStreamReader(p.getInputStream()));
            	
            	//int ret = new Integer(in.readLine()).intValue();
            	
            	p.waitFor();
            	CognitoHelper helper = new CognitoHelper();
     	
            	while (in.ready())
            	{
            		String result = in.readLine();
            		System.out.println(result);
            	
            		int position = result.indexOf("Thing ID:");
            	
            		//System.out.println("position=" +position);
            		if (position > 0)
            		{
						found = true;
            			System.out.println(result.substring(position+10));
            		
            			updateDBTable(result.substring(position+10), cognito_uuid, "SensorBoardAcctTable");
            			String thingIDList = scanDBTableWithAcctID(cognito_uuid, "SensorBoardAcctTable");
            		
            			scenetitle.setText(thingIDList);
						register_message.setText("Success");
            		}
				
            	}
				if (found == false)
				{
					register_message.setText("Fail to register");
				}
            	
            	}catch(Exception err){}
        	
        	
        });


		clearThingButton.setOnAction(e -> {
            //window.close();
        	try{

				deleteItemDBTable(cognito_uuid, "SensorBoardAcctTable");
        		String thingIDList = scanDBTableWithAcctID(cognito_uuid, "SensorBoardAcctTable");
            		
            		scenetitle.setText(thingIDList);
            	
            	}catch(Exception err){}
        	
        	
        });

        window.setScene(scene);
        window.showAndWait();
    }
     public static void setCredentials(Credentials credentials) {
		accessKey = credentials.getAccessKeyId();
		secretKey = credentials.getSecretKey();
		sessionToken = credentials.getSessionToken();

	 }
     private static String scanDBTableWithAcctID(String cognitoUuid, String tableName) {

     	System.out.println("scanDBTableWithAcctID() In");


		BasicSessionCredentials awsCreds = new BasicSessionCredentials(accessKey, secretKey, sessionToken);
		AmazonDynamoDB ddb = AmazonDynamoDBClientBuilder.standard()
						.withRegion("us-east-1")
                        .withCredentials(new AWSStaticCredentialsProvider(awsCreds))
                        .build();
     	
     	DynamoDB dynamoDB = new DynamoDB(ddb);
 
         Table table = dynamoDB.getTable(tableName);

         Map<String, Object> expressionAttributeValues = new HashMap<String, Object>();
         expressionAttributeValues.put(":uuid", cognitoUuid);

         ItemCollection<ScanOutcome> items = table.scan("cognitoUUID = :uuid", // FilterExpression
             "thingID, cognitoUUID, deviceName", // ProjectionExpression
             null, // ExpressionAttributeNames - not used in this example
             expressionAttributeValues);

         System.out.println("Scan of " + tableName + " for items with a price less than 100.");
         Iterator<Item> iterator = items.iterator();
         
         StringBuilder thingIdList = new StringBuilder();
		 thingIdList.append("Cognito UUID=" + cognitoUuid);
		 thingIdList.append("\n");
		 thingIdList.append("\n");
         thingIdList.append("==============Thing ID===================== \n");
         
         int idx = 1;
         while (iterator.hasNext()) {
             //System.out.println(iterator.next().toJSONPretty());
         	Item item = iterator.next();
         	thingIdList.append(idx + ": " + item.get("thingID")+"\n");
             System.out.println(item.get("thingID"));
             idx++;
         }
         thingIdList.append("==========================================\n");
         
         return thingIdList.toString();
       
     }
     
     private static void updateDBTable(String thingId, String cognitoUuid, String tableName) {

      	System.out.println("UpdateItem...");
      	
      	HashMap<String,AttributeValue> item_key =
      		   new HashMap<String,AttributeValue>();

      		item_key.put("thingID", new AttributeValue(thingId));

      		HashMap<String,AttributeValueUpdate> updated_values =
      		    new HashMap<String,AttributeValueUpdate>();

      		
      		updated_values.put("cognitoUUID", new AttributeValueUpdate(
      		                new AttributeValue(cognitoUuid), AttributeAction.PUT));
      	
			BasicSessionCredentials awsCreds = new BasicSessionCredentials(accessKey, secretKey, sessionToken);
			AmazonDynamoDB ddb = AmazonDynamoDBClientBuilder.standard()
						.withRegion("us-east-1")
                        .withCredentials(new AWSStaticCredentialsProvider(awsCreds))
                        .build();

      	

      		try {
      		    ddb.updateItem(tableName, item_key, updated_values);
      		} catch (ResourceNotFoundException e) {
      		    System.err.println(e.getMessage());
      		    System.exit(1);
      		} catch (AmazonServiceException e) {
      		    System.err.println(e.getMessage());
      		    System.exit(1);
      		    
      		}
      		
      		return;
      		    	

      		    
        
      }

	private static void deleteItemDBTable(String cognitoUuid, String tableName) {

		System.out.println("DeleteItem: ...");


		BasicSessionCredentials awsCreds = new BasicSessionCredentials(accessKey, secretKey, sessionToken);
		AmazonDynamoDB ddb = AmazonDynamoDBClientBuilder.standard()
						.withRegion("us-east-1")
                        .withCredentials(new AWSStaticCredentialsProvider(awsCreds))
                        .build();
 
     	DynamoDB dynamoDB = new DynamoDB(ddb);
 
         Table table = dynamoDB.getTable(tableName);

         Map<String, Object> expressionAttributeValues = new HashMap<String, Object>();
         expressionAttributeValues.put(":uuid", cognitoUuid);

         ItemCollection<ScanOutcome> items = table.scan("cognitoUUID = :uuid", // FilterExpression
             "thingID, cognitoUUID, deviceName", // ProjectionExpression
             null, // ExpressionAttributeNames - not used in this example
             expressionAttributeValues);

         
         Iterator<Item> iterator = items.iterator();
         
        
         
         int idx = 1;
         while (iterator.hasNext()) {
             //System.out.println(iterator.next().toJSONPretty());
         	Item item = iterator.next();
         	DeleteItemOutcome outcome = table.deleteItem("thingID", item.get("thingID"));
            System.out.println("delete " + item.get("thingID"));
            idx++;
         }
   
      	return;
        
    }
}
