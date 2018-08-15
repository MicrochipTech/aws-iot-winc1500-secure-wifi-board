package com.amazonaws.sample.cognitoui;

import com.amazonaws.AmazonServiceException;
import com.amazonaws.services.cognitoidentity.model.Credentials;
import com.amazonaws.services.dynamodbv2.AmazonDynamoDB;
import com.amazonaws.services.dynamodbv2.AmazonDynamoDBClientBuilder;
import com.amazonaws.services.dynamodbv2.document.DynamoDB;
import com.amazonaws.services.dynamodbv2.document.Item;
import com.amazonaws.services.dynamodbv2.document.ItemCollection;
import com.amazonaws.services.dynamodbv2.document.ScanOutcome;
import com.amazonaws.services.dynamodbv2.document.Table;
import com.amazonaws.services.dynamodbv2.model.AttributeValue;
import com.amazonaws.services.dynamodbv2.model.GetItemRequest;
import com.amazonaws.auth.BasicSessionCredentials;
import com.amazonaws.auth.AWSStaticCredentialsProvider;
import com.amazonaws.regions.Region;
import com.amazonaws.regions.Regions;


import javafx.application.Application;
import javafx.event.ActionEvent;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.stage.Stage;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import org.json.JSONObject;


public class MainForm extends Application {

    public static void main(String[] args) {
        launch(args);
    }


    static void ShowUserBuckets(Credentials credentails) {

        CognitoHelper helper = new CognitoHelper();
        String message = helper.ListBucketsForUser(credentails);

        ThingListForm.display("Cognito -  Returned Credentials", message);

    }
    
    private String scanDBTableWithAcctID(Credentials credentials, String cognitoUuid, String tableName) {

		System.out.println("GetItem: printing results...");

		BasicSessionCredentials awsCreds = new BasicSessionCredentials(credentials.getAccessKeyId(), credentials.getSecretKey(), credentials.getSessionToken());
		AmazonDynamoDB ddb = AmazonDynamoDBClientBuilder.standard()
						.withRegion("us-east-1")
                        .withCredentials(new AWSStaticCredentialsProvider(awsCreds))
                        .build();
  
		
    	DynamoDB dynamoDB = new DynamoDB(ddb);
        
        Table table = dynamoDB.getTable(tableName);

        Map<String, Object> expressionAttributeValues = new HashMap<String, Object>();
        expressionAttributeValues.put(":uuid", cognitoUuid);

        ItemCollection<ScanOutcome> items = table.scan("cognitoUUID = :uuid", // FilterExpression
            "thingID, cognitoUUID", // ProjectionExpression
			//"deviceID", // ProjectionExpression
            null, // ExpressionAttributeNames - not used in this example
            expressionAttributeValues);

        System.out.println("Scan of " + tableName + " for item with Cognito UUID " + cognitoUuid);
        Iterator<Item> iterator = items.iterator();
        
        StringBuilder thingIdList = new StringBuilder();

		thingIdList.append("Cognito UUID=" + cognitoUuid + "\n");
		thingIdList.append("\n");
        thingIdList.append("==============Thing ID===================== \n");
        
       
       
        
        int idx = 1;
        while (iterator.hasNext()) {
          //  System.out.println(iterator.next().toJSONPretty());
        	Item item = iterator.next();
        	thingIdList.append(idx + ": " + item.get("thingID")+"\n");
            System.out.println(item.get("thingID"));
            idx++;
        }
        thingIdList.append("==========================================\n");
        
        return thingIdList.toString();
        
    }

    @Override
    public void start(Stage primaryStage) {

        CognitoHelper helper = new CognitoHelper();
        Stage window;
        Button signup_button;
        Button signin_button;
        Button forgot_pswd_button;
		

        window = primaryStage;
        window.setTitle("Secure Wi-Fi Board Workshop");
        VBox vb = new VBox();
        vb.setPadding(new Insets(10, 50, 50, 50));
        vb.setSpacing(10);
        // Username field
        TextField Username = new TextField();
        Label username_label = new Label("Username:");
        HBox hbu = new HBox();
        hbu.getChildren().addAll(username_label,Username);
        hbu.setSpacing(10);
        // password field
        TextField Password = new PasswordField();
        Label password_label = new Label("Password:");
        HBox hbp = new HBox();
        hbp.getChildren().addAll(password_label,Password);
        hbp.setSpacing(10);

        signup_button = new Button("Sign-Up");
        vb.setPadding(new Insets(10, 50, 50, 50));
        vb.setSpacing(10);
        Label lbl = new Label("");
        Image image = new Image(getClass().getClassLoader().getResourceAsStream("microchip_master_workshop.png"));
        lbl.setGraphic(new ImageView(image));
        lbl.setTextFill(Color.web("#0076a3"));
        lbl.setFont(Font.font("Amble CN", FontWeight.BOLD, 24));
        vb.getChildren().add(lbl);
        signup_button.setOnAction(e -> {
            boolean result = ConfirmBox.display("Secure Wi-Fi board Workshop", "Sign-Up Form");
            System.out.println(result);
        });
        signin_button = new Button("Sign-In");
        Label auth_message = new Label("");
        signin_button.setOnAction((ActionEvent e) -> {
			String congito_uuid;
            String result = helper.ValidateUser(Username.getText(), Password.getText());
            if (result != null) {
				congito_uuid = CognitoJWTParser.getPayload(result).getString("sub");
				System.out.println("cognito uuid = " + congito_uuid);
                System.out.println("User is authenticated:" + result);
                auth_message.setText("User is authenticated");
                JSONObject payload = CognitoJWTParser.getPayload(result);
                String provider = payload.get("iss").toString().replace("https://", "");

                Credentials credentails = helper.GetCredentials(provider, result);

			
                //ShowUserBuckets(credentails);
                
				

                String thingIDList = scanDBTableWithAcctID(credentails, congito_uuid, "SensorBoardAcctTable");
				ThingListForm.setCredentials(credentails);
                ThingListForm.display("Secure Wi-Fi board Workshop", thingIDList);
            } else {
                System.out.println("Username/password is invalid");
                auth_message.setText("Username/password is invalid");
            }

        });
        forgot_pswd_button = new Button("Forgot Password?");
        forgot_pswd_button.setOnAction(e -> {
            boolean result = ForgotPassword.display("Secure Wi-Fi board Workshop", "Forgot password");
            System.out.println(result);
        });
        signup_button.setMaxWidth(142);
        signin_button.setMaxWidth(142);
        forgot_pswd_button.setMaxWidth(142);


        Hyperlink hl = new Hyperlink("Cognito Hosted UI");
        hl.setTooltip(new Tooltip(helper.GetHostedSignInURL()));
        hl.setOnAction((ActionEvent event) -> {
//            Hyperlink h = (Hyperlink) event.getTarget();
//            String s = h.getTooltip().getText();
//            this.getHostServices().showDocument(s);
//            event.consume();
            HostedUI.display("Secure Wi-Fi board Workshop", helper.GetHostedSignInURL());

        });
/* StackPane layout = new StackPane(); */
        vb.getChildren().addAll(hbu, hbp, signin_button, signup_button, forgot_pswd_button, auth_message, hl);
        vb.setAlignment(Pos.CENTER);


        Scene scene = new Scene(vb, 400, 500);
        window.setScene(scene);
        window.show();
    }


}
