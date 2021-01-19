//-----------------------------------------------------------
//--Date	: December 4, 2020
//--Purpose : Assignment 2 DBS311
//-----------------------------------------------------------
//Name 								Section 		Seneca ID
//-----------------------------------------------------------
//Fiona Taniredjo 					NDD 			027294032
//Zulfiyya Farhadli 				NGG 			168707180
//Jigarkumar Pritamkumar Koradiya 	NDD 			164861189
//Kyle LeBlanc 						NEE 			112337191
//Brooke Isbister 					NEE 			166344184
//Luan Lima Campos 					NDD 			119386191
//-----------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <occi.h>

using oracle::occi::Environment;
using oracle::occi::Connection;
using namespace oracle::occi;
using namespace std;

struct ShoppingCart {
	int product_id;
	double price;
	int quantity;
};

int mainMenu(void) {
	int value = 0;
	do {
		cout << "************* Main Menu by Koradiya ********************" << endl;
		cout << "1)      Login" << endl;
		cout << "0)      Exit" << endl;
		if (value != 0 && value != 1) {
			cout << "You entered a wrong value. ";
		}
		cout << "Enter an option (0-1): ";
		cin >> value;
		if (!cin) {
			cin.clear();
			cin.ignore(1000, '\n');
			value = -1;
		}
	} while (value != 0 && value != 1);

	return value;
}

int customerLogin(Connection* conn, int customerId) {
	int result;
	try {
		Statement* stmt = conn->createStatement();
		stmt->setSQL("BEGIN find_customer(:1, :2); END;");
		stmt->setInt(1, customerId);
		stmt->registerOutParam(2, Type::OCCIINT);
		stmt->executeUpdate();
		result = stmt->getInt(2);
		conn->terminateStatement(stmt);
	}
	catch (SQLException & sqlExcp) {
		cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
	}
	//returns 1 if the customer exists - returns 0 if the customer does not exist
	return result;
}

double findProduct(Connection* conn, int product_id) {
	double price;
	try {
		Statement* stmt = conn->createStatement();
		stmt->setSQL("BEGIN find_product(:1, :2); END;");
		stmt->setInt(1, product_id);
		stmt->registerOutParam(2, Type::OCCIINT);
		stmt->executeUpdate();
		price = stmt->getInt(2);
		conn->terminateStatement(stmt);
	}
	catch (SQLException & sqlExcp) {
		cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
	}
	//returns price if product exists or else 0
	return price;
}

int addToCart(Connection* conn, struct ShoppingCart cart[]) {
	int prodID;
	int quantity;
	double result;
	int count = 0;
	cout << "-------------- Add Products to Cart --------------" << endl;
	do {
		if (count < 5) {
			cout << "Enter the product ID: ";
			cin >> prodID;
			if (!cin) {
				cin.clear();
				cin.ignore(1000, '\n');
			}
			result = findProduct(conn, prodID);
			if (result == 0) {
				cout << "The product does not exist. Try again..." << endl;
			}
			else {
				cart[count].product_id = prodID;
				cart[count].price = result;
				cout << "Product Price: " << cart[count].price << endl;
				do {
					cout << "Enter product quantity: ";
					cin >> quantity;
					if (!cin) {
						cin.clear();
						cin.ignore(1000, '\n');
						quantity = 0;
					}
				} while (quantity < 1);
				cart[count].quantity = quantity;
				count++;
				do {
					cout << "Enter 1 to add more products or 0 to checkout: ";
					cin >> result;
					if (!cin) {
						cin.clear();
						cin.ignore(1000, '\n');
						result = -1;
					}
				} while (result != 1 && result != 0);
				result = !result;
			}
		}
		else {
			cout << "Maximum of 5 items per cart. Proceeding to checkout." << endl;
			result = -1;
		}
	} while (result == 0);
	return count;
}

void displayProducts(struct ShoppingCart cart[], int productCount) {
	double total = 0;
	cout << "------- Ordered Products ---------" << endl;
	for (int i = 0; i < productCount; i++) {
		cout << "---Item " << i + 1 << endl;
		cout << "Product ID: " << cart[i].product_id << endl;
		cout << "Price: " << cart[i].price << endl;
		cout << "Quantity: " << cart[i].quantity << endl;
		total += (cart[i].quantity * cart[i].price);
	}
	cout << "----------------------------------" << endl;
	cout << "Total: " << fixed << setprecision(2) << total << endl;
}

int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount) {
	string result;
	bool flag;
	int order_no;
	do {
		cout << "Would you like to checkout? (Y/y or N/n) ";
		cin >> result;
		if (result == "Y" || result == "y") {
			flag = false;
			try {
				Statement* stmt = conn->createStatement();
				stmt->setSQL("BEGIN add_order(:1, :2); END;");
				stmt->setInt(1, customerId);
				stmt->registerOutParam(2, Type::OCCIINT);
				stmt->executeUpdate();
				order_no = stmt->getInt(2);
				conn->terminateStatement(stmt);
			}
			catch (SQLException & sqlExcp) {
				cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
			}
			for (int i = 0; i < productCount; i++) {
				try {
					Statement* stmt = conn->createStatement();
					stmt->setSQL("BEGIN add_orderline(:1, :2, :3, :4, :5); END;");
					stmt->setInt(1, order_no);
					stmt->setInt(2, i + 1);
					stmt->setInt(3, cart[i].product_id);
					stmt->setInt(4, cart[i].quantity);
					stmt->setDouble(5, cart[i].price);
					stmt->executeUpdate();
					conn->terminateStatement(stmt);
				}
				catch (SQLException & sqlExcp) {
					cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
				}
			}
			cout << "The order is successfully completed." << endl;
		}
		else if (result == "N" || result == "n") {
			cout << "The order is cancelled." << endl;
			flag = false;
		}
		else {
			cout << "Wrong input. Try again..." << endl;
			flag = true;
		}
	} while (flag);
	return flag;
}


int main(void) {
	//declare the environment and the connection variables
	Environment* env = nullptr;
	Connection* conn = nullptr;
	//Define and initialize variables to store username, password, and host address
	string user = "dbs311_203d18";
	string pass = "10496178";
	string constr = "myoracle12c.senecacollege.ca:1521/oracle12c";

	//establish connection
	try {
		env = Environment::createEnvironment(Environment::DEFAULT);
		conn = env->createConnection(user, pass, constr);
	}
	catch (SQLException & sqlExcp) {
		cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
	}
	//main function
	int option;
	int custID;
	int prodCount;
	struct ShoppingCart* shoppingCart = nullptr;
	shoppingCart = new ShoppingCart[5];
	do {
		option = mainMenu();
		if (option == 1) {
			cout << "Enter the customer ID: ";
			cin >> custID;
			if (!cin) {
				cin.clear();
				cin.ignore(1000, '\n');
				cout << "You entered a wrong value. " << endl;
			}
			else {
				if (customerLogin(conn, custID) == 1) {
					prodCount = addToCart(conn, shoppingCart);
					displayProducts(shoppingCart, prodCount);
					option = !checkout(conn, shoppingCart, custID, prodCount);
				}
				else {
					cout << "The customer does not exist." << endl;
				}
			}
		}
	} while (option != 0);
	//remove connection and delete memory
	env->terminateConnection(conn);
	Environment::terminateEnvironment(env);
	delete[] shoppingCart;
	return 0;
}

/*		// SQL
//find_customer
CREATE OR REPLACE PROCEDURE find_customer(customer_id IN Number, found OUT Number) AS
inputCust customers% ROWTYPE;
BEGIN
SELECT* INTO inputCust FROM customers
WHERE cust_no = customer_id;
found: = 1;
EXCEPTION
WHEN TOO_MANY_ROWS
THEN
DBMS_OUTPUT.PUT_LINE('Too Many Rows Returned.');
WHEN NO_DATA_FOUND
THEN
found : = 0;
DBMS_OUTPUT.PUT_LINE(customer_id || ' Not Found.');
WHEN OTHERS
THEN
DBMS_OUTPUT.PUT_LINE('ERROR');
END find_customer;

//find_product
CREATE OR REPLACE PROCEDURE find_product(product_id IN Number, price OUT products.prod_sell% TYPE) AS
inputProd products% ROWTYPE;
BEGIN
SELECT* INTO inputProd FROM products
WHERE prod_no = product_id;
price: = inputProd.prod_sell;
EXCEPTION
WHEN TOO_MANY_ROWS
THEN
DBMS_OUTPUT.PUT_LINE('Too Many Rows Returned.');
WHEN NO_DATA_FOUND
THEN
price : = 0;
DBMS_OUTPUT.PUT_LINE(product_id || ' Not Found.');
WHEN OTHERS
THEN
DBMS_OUTPUT.PUT_LINE('ERROR');
END find_product;

//add_order
CREATE OR REPLACE PROCEDURE add_order(customer_id IN Number, new_order_id OUT Number) AS
inputOrdNo orders.order_no% TYPE;
BEGIN
SELECT max(order_no) INTO inputOrdNo FROM orders
WHERE customer_id = cust_no;
new_order_id: = inputOrdNo + 1;

INSERT INTO orders
(order_no, cust_no, status, rep_no, order_dt)
VALUES
(new_order_id, customer_id, 'C', 36, to_char(sysdate, 'dd-Mon-yyyy'));

EXCEPTION
WHEN TOO_MANY_ROWS
THEN
DBMS_OUTPUT.PUT_LINE('Too Many Rows Returned.');
WHEN NO_DATA_FOUND
THEN
DBMS_OUTPUT.PUT_LINE(customer_id || ' Not Found.');
WHEN OTHERS
THEN
DBMS_OUTPUT.PUT_LINE('ERROR');
END add_order;

//add_orderline
CREATE OR REPLACE PROCEDURE add_orderline(orderId IN orderlines.order_no% type, itemId IN orderlines.line_no% type, productId IN orderlines.prod_no% type, quantity IN orderlines.qty% type, price IN orderlines.price% type) AS
BEGIN
INSERT INTO orderlines
(order_no, line_no, prod_no, price, qty)
VALUES
(orderId, itemId, productId, price, quantity);

EXCEPTION
WHEN OTHERS
THEN
DBMS_OUTPUT.PUT_LINE('ERROR');
END add_orderline;

//query orders and orderlines from 2020
select* from orders where order_dt like '%2020';
select* from orderlines where order_no in(select order_no from orders where order_dt like '%2020');
*/