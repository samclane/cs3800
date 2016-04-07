#!/bin/bash

ancestry(){
    echo "THE ANCESTRY TREE FOR THE CURRENT PROCESS IS..."
    echo ""
    myPPID=$$
    echo $myPPID
    while [ $myPPID -ne 1 ]; do
        echo "  |"
        myPPID=$( ps -ef | awk -v var="$myPPID" ' $2 == var { print $3 } ' )  
        echo $myPPID
    done
}

process_user () {
    echo "Select a currently online user to view his processes:"
    count=1
    for user in $(who | awk '{ print $1} ' | uniq); do
        echo "$count) $user"
        count=${count+1}
    done
    read userchoice
    user=$(who | awk '{ print $1 }' | uniq | awk -v var="$userchoice" 'NR==var;START{print}')
    echo ""
    ps -ef | awk -v var="$user" '$1 ~ var {print}'
    echo ""
    
}

myexit=0
while [ $myexit -eq 0 ]; do
    echo "WELCOME"
    echo "---------------------------------------------------------"
    echo "select a menu option"
    echo "1- ancestry history"
    echo "2- who is online"
    echo "3- what process any user is running"
    echo "4- exit" 
    read choice
    echo ""
    case $choice in
        1 ) ancestry
        ;;
        2 ) who | awk '{ print $1 }' | uniq
        ;;
        3 ) process_user
        ;;
        4 ) myexit=1
        ;;
    esac
done

