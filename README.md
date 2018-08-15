# aws-iot-winc1500-secure-wifi-board
This package contains all the files (firmware code, python scripts, java pc gui) for the WINC1500 Secure Wi-Fi Board

This Secure Wi-Fi Board connect to AWS IoT, update the sensor, LED and button status to AWS Shadow. 
The board is working with Alexa custom skill.
User can speak to Echo dot to control the LED, GPIO of board and get the LED, buttons and sensor data.

## Hardware Setup

The hardware platform of this project is the WINC1500 Secure Wi-Fi Board. User can get this board in Master Conference 2018. Application firmware is already loaded to the board by default.

There is a bootloader pre-programmed to the board.
In case user need to modify the firmware,user can trigger the bootloader mode by press and hold SW1 and use SAM-BA V2.18 PC GUI to load a firmware to the board through USB port.
Or user can use Atmel-ICE Debugger to load firmware to the board directly

## Firmware Setup

Folder mcu-firmware/ contain the source code of the MCU firmware.
There are 2 project solution files

saml21g18b_sensor_board_demo_ECC.atsln - This is the project used to perform ECC608 configurion for AWS provision

saml21g18b_sensor_board_demo_JITR.atsln - This is the project for the demo application. Board running this firmware connect to AWS IoT, publish and subscribe AWS Shadow Topics

To make the firmware working with the bootloader, starting address is set as 0x2000 in current project.
If user user Atmel ICE to program the firmware wihtout bootloader, starting address is need to set back to 0x0

Steps:

Open the solution project in Atmel Studio -> Right click the solution file and select "Properties" -> Select Toolchain -> Select "ARM/GNU Linker" -> Select Miscellaneous --> Remove "-Wl,--section-start=.text=0x2000," in the Linker Flags

## AWS Setup

User can create a AWS account
https://aws.amazon.com/

**AWS IoT**

To set up AWS IoT Cloud, User can follow the user guide of AWS Zero Touch Provisioing Kit project
(From Section 2 Software Installation to Section 5 AWS IoT Just-In-Time Registration Setup )

http://microchipdeveloper.com/iot:ztpk


**Amazon Cogito Setup**

Amazon Cogito is used to provide user identity feature. User can use a java pc gui to sign-up, sign-in the account and control their boards.

Below is the Step to set up Amazon Cognito:
1) Go to Amazon Cognito Console in AWS cloud
	https://console.aws.amazon.com/cognito/ 
2) Click on "Manage your User Pools" to open Your User Pools browser.
3) Create User Pool
4) Create App Client "WiFISecureBoardForJavaApp" in the User Pool just created
5) Ensure un-tick Generate client secret when create App Client
6) In App Client setting of "WiFISecureBoardForJavaApp", tick Cognito User Pool, Input "http://microchip.com" in Callback URL. Also tick Authorization code grant, phone, email, openid
7) Create Another App Client "WiFISecureBoardForAlexa" in the User Pool just created
8) Ensure tick Generate client secret when create App Client
9) In App Client setting of "WiFISecureBoardForAlexa", tick Cognito User Pool. Also tick Authorization code grant, phone, email, openid, aws.cognito.signin.user.admin, profile
10) Fill in Callback URL with the Re-direct URL that find in the Alexa Skill setting.
11)Create Identity Pool
12) In Authentication providers of the Identity pool setting, select Cognito, and fill in the user pool ID and the App client ID (WiFISecureBoardForJavaApp)
12) When you creat the identity pool, Cognito will setup two roles in Identity and Access Management (IAM). These will be named something similar to: Cognito_<<PoolName>>Auth_Role and Cognito_<<PoolName>>Unauth_Role. You can view them by pressing the View Details button on the console. Now press the Allow button to create the roles.

we will attach a policy to the unauthenticated role to setup permissions to access the required AWS IoT APIs. This is done by first creating the IAM Policy shown below in the IAM Console and then attaching it to the unauthenticated role. In the IAM console, Search for the pool name that you created and click on the link for the unauth role. Click on the "Add inline policy" button and add the following policy using the JSON tab. Click on "Review Policy", give the policy a descriptive name and then click on "Create Policy". This policy allows the sample app to create a new certificate (including private key) and attach a policy to the certificate.

	```
   	{
  		"Version": "2012-10-17",
  		"Statement": [
    		{
      			"Effect": "Allow",
      			"Action": [
        			"iot:AttachPrincipalPolicy",
        			"iot:CreateKeysAndCertificate"
      			],
      			"Resource": [
        			"*"
      			]
    		}
  		]
	}
        ```

**Amazon Dynamodb**

Dynamodb Table is used to store the user account ID and the Secure Wi-Fi Board Thing ID. When Alexa send directives to Lambda function , Lambda function scan this table to control the correspond Thing Shadow.

Steps:
1) Go to Amazon Cognito Console in AWS cloud
	https://console.aws.amazon.com/dynamodb
2) Click on "Create Table" 
3) Create Table with the name "SensorBoardAcctTable", and primary key is "thingID" (String)


**Amazon Lambda**

A Lambda function need to be set up to process the directives receive from Alexa Skill.

Folder \lambda-function\alexa-smart-home-skill are the lambda function code.

Steps:
1. Go to AWS Lambda Console in AWS cloud
	https://console.aws.amazon.com/lambda
2. Click on "Create Function"
3. Select "Author from scratch" and fill in a name for the Lambda function. Select "Node.js 6.10" for "Runtime"
4. Select a Role for the Lambda function 
	(user can create a Role in IAM, the role need to have policy "AWSLambdaFullAccess" and "CloudWatchLogsFullAccess")
5. After Create the Lambda function , user can upload the function code. As the fucntion code inlcude a number of files, user can upload the code as a zip file
	- Select "Upload a .ZIP file" at "Code entry type"
	- Select \lambda-function\alexa-smart-home-skill\alexa-smart-home-skill.zip and upload
6. Add triggers "Alexa Skill Kit", and fill in the skill ID


## Alexa Skill Setup
As this project can support Alexa, user need to create Alexa custom skill in https://developer.amazon.com
We have a class in Master Conference 2018 which talk about the way to creat Alexa custom skill step by step. User can get the information of that class to have more idea on how to create the Alexa custom skill.

Folder alexa-skill/ contain the customer skill json file for this project

Below is the account linking of the skill setting:
Authorization URI: https://<Domain_Name_Of_Cognito_User_Pool>/login
Access Token URI: https://<Domain_Name_Of_Cognito_User_Pool>/token
Client ID: <User_Pool_Client_App_ID>
Client Secret: <User_Pool_Client_App_Secret>
Client Authentication Scheme:  HTTP Basic
Scope: profile

## AWS Provision Setup
The board need to be register to AWS IoT. To do this, AWS Provisioning steps need to be perform.
Device certificate and private key are generated and stored in the ECC608 on the board.
CA certification is registered to AWS IoT

**Steps**

A) Run Python script "_CreateCertsAndRegister2AWS.py" 

IMPORTANT:  Make sure you provide your account profile fro "AWS CLI" to configure your AWS IoT account

	1)  You create a certCA (rootCA.crt) with its private key (root-ca.key,  if this one already exist,  it will reuse it)
	
	2)  You then create a signing certificate (signer-ca.csr)and its private key (signer-ca.key, if this one already exist,  it will reuse it)
	
	3)  The root-ca  will then sign the signer-ca.csr and create signer-ca.crt
	
	4)  This signer-ca.crt is then uploaded to your AWS IoT account
	
B) Program the ECC.bin code
	
C) Python script "_Commission_WiFi_ECC_2AWS.py"

	1)  Script commission the ECC508 with SSID and PASSWORD for the WiFi connection.
	
	2)  Script request the ECC508 to generate a signing certificate (with its private key remaining private in the ECC)
	
	3)  The script receives the CSR and signs it with the signer-ca private key signer-ca.key
	
	4)  Script sends it back to the ECC508 that stores it.
	
D) Program the JITR.bin project 
	
E) If commissionong OTHER Sensor Board, just run B,C,D again.  No need to creaet a new CERT

