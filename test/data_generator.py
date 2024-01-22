import json
import random

def generate_random_data(num_records):
    records = []
    for i in range(1, num_records + 1):
        ord_id = f"OrdId{i}"
        sec_id = f"SecId{random.randint(1, 10)}"
        transaction_type = random.choice(['Sell', 'Buy'])
        amount = random.randint(0, 4294967295)
        user = f"User{random.randint(1, 20)}"
        company = f"Company{random.randint(1, 3)}"
        record = {
            "OrdId": ord_id,
            "SecId": sec_id,
            "TransactionType": transaction_type,
            "Amount": str(amount),  # Converting to string to handle large numbers
            "User": user,
            "Company": company
        }
        records.append(record)
    return records

# Define the number of records you want to generate
num_records = 1000000  # Change this number as needed

# Generate random data
random_data = generate_random_data(num_records)

# Convert to JSON
json_data = json.dumps(random_data, indent=4)

# Write to a file
with open('random_data_1.json', 'w') as file:
    file.write(json_data)

print(f"{num_records} records have been written to random_data.json")
