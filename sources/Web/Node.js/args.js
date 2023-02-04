var args = process.argv;
console.log(args);
console.log('B');

if(args[0] == '1') {
    console.log('C');
}
else {
    console.log('D');
}