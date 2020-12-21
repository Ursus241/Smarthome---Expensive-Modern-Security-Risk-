secret = 346
guess = 0
i = 0

while guess != secret:
    guess = int( input("Guess") )
    if guess < secret:
        print ("Too small")
    if guess > secret:
        print ("Too big")
    i = i + 1
print ("You did it at the ",i," try. Congratulations")
    
