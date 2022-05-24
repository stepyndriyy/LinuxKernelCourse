echo "add oleg 1010 aa" > /dev/handbook
echo "add ivan 1011 va" > /dev/handbook
echo "get oleg" > /dev/handbook 
head -c17 /dev/handbook
echo "add a 1011 va" > /dev/handbook
echo "add b 1011 va" > /dev/handbook
echo "add c 1011 va" > /dev/handbook
echo "add d 1011 va" > /dev/handbook

echo "del ivan" > /dev/handbook 
echo "del d" > /dev/handbook # del first item
echo "del oleg" > /dev/handbook # del last item

echo "get b" > /dev/handbook
head -c17 /dev/handbook
