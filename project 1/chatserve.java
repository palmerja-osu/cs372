/***********************************************
Author:  James Palmer
Project 1
Description: Implement a client-server network
  application using the sockets API. 
***********************************************/


import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.Scanner;
import java.io.IOException;
import java.io.Console;
import java.io.BufferedReader;
import java.io.PrintWriter;
import java.net.*;

public class chatserve {
  public static void main(String[] args) throws IOException {
   

   // Check for correct number of arguments - 1! 
    if (args.length !=1) {
      System.err.println("ERROR:  { java chatserve <portNumber> }");
      System.exit(1);
    }

    // acquire port information 
    int serverPortNumber = Integer.parseInt(args[0]);
    ServerSocket serverSocket = new ServerSocket(serverPortNumber);

    // print out server info 
    System.out.println("\nWelcome to Project 1 ChatServe ");
    System.out.println("Port: " + serverPortNumber + " ChatServe is now connected! Congratulations! \n");

    // User name prompt with error check 
    Console userConsole = System.console();
    System.out.println("\nUser names may not be longer than 10 characters long and may not have any spaces.\n");
    String userHandle = userConsole.readLine("Enter your chat handle: ");

    while((userHandle.length() > 10) || (containsWhiteSpace(userHandle))) {
      System.out.println("\nError: User handles may not be longer than 10 characters long and may not have any spaces.  Please try again...\n");
      userHandle = userConsole.readLine("Enter your chat handle: ");
    }

    // Clean up output with username in file header 
    String chatHeader = userHandle + "> ";

    // start connection check 
    while(true) {
      
      Socket clientSocket = serverSocket.accept();
      System.out.println("\nConnection established!");
      System.out.println("\nTo end connection type '\\quit'\n");

      String message = " ";
      String responseMsg;
      String clientQuit = "\\quit";
      String serverQuit = "\\quit";

      // input & output 
      BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
      PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true);

      // create scanner to get messages from server -> client 
      Scanner serverMessage = new Scanner(System.in);

      // loop until \\quit command 
      while((responseMsg = in.readLine()) != null) {
        if(responseMsg.equals(clientQuit)){
          System.out.println("//quit command used.\n");
		  System.out.println("Connection aborted by user.\n");
          System.out.println("Port: " + serverPortNumber + " chatserve is currently on standby! \n");
          // close connection to prevent faults
          out.close();
          in.close();
          clientSocket.close();

          break;
        }

        // print serve response
        System.out.println(responseMsg); 
		// print chat handle
        System.out.print(chatHeader); 
        message = serverMessage.nextLine(); 

        // keep messages to under 500 characters long 
        while((message.length() > 500)) {
          System.out.println("\nNotice: Messages must be less than 500 characters long.  Please try again\n");
          System.out.print(chatHeader); // chat handle
          message = serverMessage.nextLine(); // server message
        }

        // Send the chat header and the message to the client 
        out.println(chatHeader + message);

        if(message.equals(serverQuit)){
          System.out.println("//quit command used.\n");
		  System.out.println("Connection aborted by user.\n");
          System.out.println("Port: " + serverPortNumber + " chatserve is currently on standby!\n");

          // close connection to prevent faults
          out.close();
          in.close();
          clientSocket.close();

          break;
        }
      }
    }
  }

//added a whitespace section to visually clean up the output
//fixes issues where entire server message was clumped together

  public static boolean containsWhiteSpace(final String whitespaceCode) {
    if (whitespaceCode != null) {
      // Iterates through each character in String whitespaceCode
      for (int i = 0; i < whitespaceCode.length(); i++) {
        if (Character.isWhitespace(whitespaceCode.charAt(i))) {
          // Return if whitespace is found
          return true;
        }
      }
    }
    // return false if no whitespace is found
    return false;
  }

}