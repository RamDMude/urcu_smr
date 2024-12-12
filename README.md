# urcu_smr

To compile the code:

```make```

To run:

```./rcu_app```

To run the executable using heap profiler

```sudo apt-get install valgrind```

```valgrind --tool=massif ./rcu_app```

The output can be viewed by 
```ms_print massif.out.<processID>```
