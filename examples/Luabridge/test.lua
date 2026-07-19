firstVector = Vector(11, 14, 15);

print("call GetX on firstVector " .. firstVector.GetX());

print("call GetCoords on firstVector ", table.unpack{firstVector.GetCoords(0, 0, 0)});


secondVector = Vector(1, 5, 9);


secondVector.Print();

secondVector.SetX(119);

secondVector.Print();

firstVector.Print();

firstVector.Set(secondVector);

firstVector.Print();


queueVector = Vector(1,9,1);


queueVector.Print();

queueVector = nil;

collectgarbage();

print("garbage collected");

firstVector.Print();
secondVector.Print();

UseString(MyString("This is a test of passing across a C++ object from a function return value to another function"));

function callMeFromC(a, b) 
	print("I am a LUA function that was called from a C program with arguments a=" .. a ..  " b=" .. b); 
	return 12; 
end;



yo("Somebody", 12, callMeFromC);


print("pcall result: ", unpack{pcall(throwTestException, 12)});

print("Showing object details firstVector table:");

for i, j in pairs(firstVector) do
	print ("member " .. i .. " of type " .. type(j) .. " = " , j);
end
