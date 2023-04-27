# Client-server application in С
## _Description_

This is a simple CRUD application for working with users. The server side uses: SQLite, PTHREAD.

## _Requests_

In the client part, you can use the following queries:

| Request | Response |
| ------ | ------ |
| list_users | list of all users |
| get_user[id] | returns the user by id if it exists |
| update_user[id][name][age] | updates information about the user if it exists |
| add_user[name][age] | adds a user to the database |
| delete_user[id] | deletes the user if it exists |
| echo | returns the entered string |

## _Run_

You must run the client and server parts separately using the command:
``` make run```
