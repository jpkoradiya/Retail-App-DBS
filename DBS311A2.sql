/*find_customer*/
CREATE OR REPLACE PROCEDURE find_customer(customer_id IN Number, found OUT Number) AS
    inputCust customers%ROWTYPE;
BEGIN
    SELECT * INTO inputCust FROM customers
    WHERE cust_no = customer_id;
    found := 1;
EXCEPTION
WHEN TOO_MANY_ROWS
    THEN
        DBMS_OUTPUT.PUT_LINE('Too Many Rows Returned.');
WHEN NO_DATA_FOUND
    THEN
    found := 0;
        DBMS_OUTPUT.PUT_LINE(customer_id || ' Not Found.');
WHEN OTHERS
    THEN
        DBMS_OUTPUT.PUT_LINE('ERROR');
END find_customer;
/

/*find_product*/
CREATE OR REPLACE PROCEDURE find_product(product_id IN Number, price OUT products.prod_sell%TYPE) AS
    inputProd products%ROWTYPE;
BEGIN
    SELECT * INTO inputProd FROM products
    WHERE prod_no = product_id;
    price := inputProd.prod_sell;
EXCEPTION
WHEN TOO_MANY_ROWS
    THEN
        DBMS_OUTPUT.PUT_LINE('Too Many Rows Returned.');
WHEN NO_DATA_FOUND
    THEN
    price := 0;
        DBMS_OUTPUT.PUT_LINE(product_id || ' Not Found.');
WHEN OTHERS
    THEN
        DBMS_OUTPUT.PUT_LINE('ERROR');
END find_product;
/
/*add_order*/
CREATE OR REPLACE PROCEDURE add_order(customer_id IN Number, new_order_id OUT Number) AS
inputOrdNo orders.order_no%TYPE;
BEGIN
    SELECT max(order_no) INTO inputOrdNo FROM orders
    WHERE customer_id = cust_no;
    new_order_id := inputOrdNo + 1;
    
    INSERT INTO orders
    (order_no, cust_no, status,rep_no, order_dt)
    VALUES
    (new_order_id, customer_id, 'C', 36, to_char(sysdate,'dd-Mon-yyyy'));
    
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
/
/*add_orderline*/
CREATE OR REPLACE PROCEDURE add_orderline(orderId IN orderlines.order_no%type, itemId IN orderlines.line_no%type, productId IN orderlines.prod_no%type, quantity IN orderlines.qty%type, price IN orderlines.price%type) AS
BEGIN  
    INSERT INTO orderlines
    (order_no, line_no, prod_no , price, qty)
    VALUES
    (orderId, itemId, productId, price, quantity);
    
EXCEPTION
WHEN OTHERS
    THEN
        DBMS_OUTPUT.PUT_LINE('ERROR');
END add_orderline;
/
/*query orders and orderlines from 2020*/
select * from orders where order_dt like '%2020';
select * from orderlines where order_no = (select order_no from orders where order_dt like '%2020');
