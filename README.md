# event-simulator

short tap, duration in microseconds
```
simulate-event click x,y duration-in-MICROSECONDS
# example at coordinate 100x200 tap for 100ms
simulate-event click 100,200 100000
```

long press, duration in microseconds
```
simulate-event click x,y duration-in-MICROSECONDS
# example at coordinate 200x500 long press for 1s
simulate-event click 200,500 1000000
```

drag, duration in microseconds
```
simulate-event drag x1,y1 duration-in-MICROSECONDS x2,y2
# example drag from coordinate 200x600 to 400x600 in 700ms
simulate-event drag 200,600 700000 400,600
```
