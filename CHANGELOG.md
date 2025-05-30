# This is an unofficial changelog I keep to document the trials and tribulations of this project

# Hurdles

## UUID handling
This was the first bump in the road that was annoying as hell. The problem was to manage UUID ownership for `Order` and `Trade` structs while trying to optimize memory (and time) as much as possible.

### v1
At the start, I had the `Order` object initialize its `id` upon creation using the `uuid_generator` and then converting this to string using the `uuids::to_string` method. Then when an order is matched, it would generate a `Trade` object that holds a `string_view` to the buyer and seller `Order`'s `id` fields while also storing its own trade `id` which is a string again generated by the `uuid_generator`.

Clearly, this comes with some problems:
1. `std::string` itself is large and the UUID string is a 36-character array which totals up to ~60 bytes (24 + 36)
2. When an `Order` is matched, it gets removed from from the `OrderBook` since there's no use for it anymore. If the user doesn't store a reference to the `Order` object, it gets deallocated along with its UUID string hence leaving the `string_view`s in `Trade` and `OrderResult` dangling. The latter is not that big of a problem but `Trade`s are stored in the `OrderBook`'s `tradeList` so that's definitely a problem.

### v2
Clearly something had to be done regarding the UUID persistence and its size overhead. The easiest solution was to just have `Trade` and `OrderResult` store copies of the UUID's they are referencing but that would mean a `Trade` would be at least 180 bytes and that is unacceptable since this is not Python.

Looking back, in a day, a blue-chip stock would have around 150K trades a day and that would only total up to 30MB worth of trades for an orderbook per day which is acceptable. But then again, why code in C++ if I'm not gonna fuss over memory optimization. Also, less copying -> faster execution and that's definitely important for HFT.

The first solution was to use an `uint64_t` based internal mapping (I'll just say int from now) for the objects and then store the UUID strings in an `idPool` map in the `OrderBook` itself. So now, instead of an `Order` and a `Trade` having a UUID string, it just holds an int that maps to a UUID string. Also, any references to a UUID can be represented by an 8 byte int instead of a 16 byte `string_view`. This way, we solve both problems from before:
1. An int is only 8 bytes which is much smaller compared to a `std::string`
2. Even if an `Order` gets deallocated, its UUID string would still be stored in the `OrderBook` so there's no risk of dangling references in `Trade` and `OrderResult` objects

In essence, `Order`s and `Trade`s would be managed internally using an int but an API would still return a UUID string when these objects are returned. Also, this means that `Order` cannot initialize a UUID in its constructor anymore. Instead, when it is placed, the `OrderBook` assigns an int internal id, generate a UUID string and store this mapping in `idPool`.

### v3
Could have called it a day there but we can do better. One thing I realized after skimming through the [uuid library](include/libraries/uuid.h) is that the `uuids::uuid` objects themselves are actually just 16-byte int arrays which is half as small as their string representations! Why bother storing UUID string when I can store the `uuids::uuid` arrays directly and that's what I did. So I swapped all UUID strings to `uuids::uuid` and we all lived happily ever after.

### v4
I soon came to realize there was an unnecessary level of indirection and internal mapping here. Why should I use an internal int mapping when I can just store pointers to the `uuids::uuid` objects instead? So instead of going to `idPool` and finding the UUID that corresponds to the int id, just dereference the pointer. Eureka!

So I removed all internal int id implementation, and make everyone just store pointers to their `uuids::uuid` in `idPool` which is now an `unordered_set` instead of an `unordered_map` since we don't need the int keys anymore and we all lived happily ever after (hopefully)

### v5
Oh how I thought I wouldn't have to update this section anymore. Something I just realized is that order books should store a list of their orders somewhere for bookkeeping. Then, somehow along the train of thought I clocked that the returned `OrderResult` after placing an order should probably be self-containing instead of holding references to the `OrderBook`'s copies of stuff.

The reason is because `OrderResult`s are not managed by the `OrderBook` and exists solely to report the result of the order (aha!). Hence, if it contains references to stuff inside the `OrderBook` like `Trade`s in `tradeList` and UUID in `idPool`, the pointers would be invalid if the `OrderBook` were to, say, flush the storage into a database. The only thing it should point to is the `remainingOrder` which is actually still in the `OrderBook` and managed by it.

Also, it is up to the user to store the `OrderResult` and they could very much not store them if they don't want to. So, they can fuss over the memory management of those copies. Hence, a large scale change to change `OrderResult` to store copies instead of pointers are done and an `orderList` is maintained in the `OrderBook`

P.S. Now I realize this isn't really related to UUID handling but whatever

### v6
Following the last point, I also just realized that the `Trade`s in `OrderResult.trades` still store pointers to the IDs stored in `idPool` and would be dangling if `idPool` were to be flushed. So I had to make two types of trades, one is for internal processing which uses UUID pointers to save memory and another is for user copy that stores the actual UUID itself for persistence.

Hence, I dived into template programming and changed `Trade` to `TradeImpl` which has a non-type template parameter `ownsUUIDs`. So now a `Trade` is a `TradeImpl<false>` and a `TradeCopy` is a `TradeImpl<true>`! Next, I needed to update `callback` (in `Order`) and `trades` (in `OrderResult`) to use `TradeCopy` instead of `Trade`.

Of course this came with a lot of rewriting the tests to reflect the new API but it wasn't that bad. I just had to make a specialized 'SFINAE copy constructor' that creates a `TradeCopy` from a `Trade`, redefine a 'SFINAE aggregate constructor' for `Trade`s to mirror the aggregate constructor and redefine the copy constructors for both. Then, the compiler took care of converting the `Trade` to `TradeCopy` when I pass it into a `trades` container.

## Float precision
I knew when I used float instead of double that it's gonna bite me in the ass some day, and today (23/5/2025) is that day

### v1
I used float in hopes of having an `Order` be 64 bytes for cache alignment but I started running into problems with precision for anything beyond 2 dp. This is a problem since penny stocks trade up to 4dp. So I had to concede the cache alignment and use double instead. This made an `Order` object 72 bytes now (68 bytes and some misalignment)

### v2
Another problem I now encounter is that getting map values using doubles don't work. Even if the `bidMap` has an entry with key 60, I can't do `bidMap.at(60)` due to precision issues since 60 is not equal 60 for some flipping reason. So now I have to use an int for internal map keys and a utility `convertTick` function that converts any double inputs to int based on the `tickSize` of the `OrderBook`

### v3
Truncating is kinda messed up in some places bcs floats are weird bro. A number like 60.05 = 60.05000495 but 60.06 = 60.05999999234 and then when I divide by tick size I get 6005.999 and it truncates to 60.05. So what I did was add a small `tickSize` correction term after the scaling so it pushes the numbers that are close to the next number (like 6005.999 + 0.01 = 6006) but doesn't disrupt numbers that are already there (like 6005.00 + 0.01 = 6005). Still, I'm suffering from that choice of converting double to float. Spent a while tweaking bcs of a float literal in a default value smh.

# Plans

## Stop Orders
Recently just realized that stop orders exist so I'll have to implement them. Doesn't seem that hard though but they do require more fields and slightly different mechanisms so here's the play
1. Add a `stopPrice` field in `Order` and `STOP_LIMIT` and `STOP` enum in `Order::Type`. Yes inheritance seems perfect here but then we would need separate vectors for auditing since storing `StopPrice` in `orderList` would slice it and remove the stop price member so I'll need another vector just to store stop orders. Also benchmarks show that adding another double member doesn't negatively impact performance. Now this would mean we need to make the constructor private since having two double parameters with default value is ambiguous. Instead we'll only expose the factory functions which sadly also means updating all tests but shouldn't be too much. Also now that there are more enums, `Order` should have convenience type checker bool functions so I don't have to write double comparisons every time.
2. `OrderBook` would store another ordered `map<price, Order>` that stores the stop orders. After a market price movement, a function should search this map either from begin or end based on price movement. I'll figure it out. Also some extra validation that stop buys are above market price and stop sells are below market price when placing.
3. When a stop order is placed, first it's stamped and a copy is stored in `orderList`. Then a (truncated) local copy goes into the `stopMap` on its (converted) price level and a pointer is put into `idMap`. I think `OrderLocation` should have a separate enum to determine which map to check instead of just using `Order::Side` now. Then when it's triggered, it gets matched and if any volume remains (if limit), it gets **moved** into the corresponding bid/ask map. The states we need to update here are
    - previous price level at `stopMap`. Remove if now empty
    - enum and pointer in `idMap`
4. Now, we have very branchy placing logic. Stop order should skip stamping when they get placed after trigger, also they don't need to be added to `idMap` (just updated) and whatever else we might discover when actually implementing this. It might seem like a good time to refactor `placeOrder` into a template and keep the public facing API to be a dispatcher similar to `matchOrder`. However, we need to solve the problem of header bloat here. I think we'll have to separate out some big logical chunks out of the bigger template functions like `matchOrderTemplate` and `placeOrderTemplate` and move them into the .cpp file.

# Thoughts

## Cache alignment
Started fussing over cache alignment here. `Order` objects are already 64 bytes (perfect!) but `Trade`s are 48 bytes but their corresponding id's are 16 bytes. Thing is, there's no reason to store the trade id's in `idPool` since no other objects point to them except the `Trade`s themselves which persists in the `tradeList`. There are two approaches I considered here
- Store `uuid` directly in `Trade` instead of a pointer. Pros: Pads up to 56 bytes, doesn't require dereferencing to get `id`. Cons: Have to change current implementation and tests
- When I get to the memory allocator part, make sure each `Trade` objects are stored next to their id in memory. Pros: Pads up exactly to 64 bytes, no need to change implementation except maybe change tests that check trade id is stored in `idPool`. Cons: Way more complicated and probably wouldn't be worth it

As I thought about the second approach more, I started to think would optimizing the `Trade` placement in memory for cache alignment be worth it? After all, `Trade`s aren't a big part of the `OrderBook` operations anyways. They are created once when `Order`s match, stored in `tradeList` for bookkeeping and sent as copies to users. After that, no further references to them are made except probably when they are flushed into a database. I'll take a look again once I've got the basic implementation done and get some benchmarks setup but this thought process is good to document so I don't have to think about it all over again in case `Trade` memory alignment do become a bottleneck for some reason.

Man now the `Order` is no longer 64 bytes. Some ways to make it work is maybe using an `int32_t` to store the price and just divide by a precision value. But this has some problems:
- If we just use a universal precision like 0.0001 (penny stocks precision) then we can just multiply the price by 0.0001 everytime we need to use it and we can live happily ever after. Thing is, BRK.A trades for ~$700k which would be ~7,000,000,000 with this system, and if you haven't realized, a 32-bit int can only store up to ~2,000,000,000 and even its unsigned counterpart can only store double that. If we use a 64-bit int then that would be just as big as a double.
- If we use a relative precision then it would solve this previous problem. If an order is 90c then its precision would be 0.0001 so it would be 900,000. If an order is $9000 then its precision would be 0.01 so it would be 900,000 as well. The question is how does an `OrderBook` tell these apart? If the `tickSize` is 0.01 or 0.0001 then sure, but what if its 0.001? 0.05? 1? These wouldn't work unless I have another member which tracks the precision which then would make the `Order` just as big as it was.

Had a crazy epiphany. Instead of the `Side` and `Type` enum with both taking 2 values each, I can just make a `Kind` enum or something like that that takes 4 values. This would knock off 4 bytes making the `Order` 64 bytes again with proper alignment. However, this would need some large scale refactoring. What I plan to do is, get the tests passing first, set up some benchmark and test if the cache alignment is actually a bottleneck by changing the price from float to double (risking some precision here but this is purely for testing performance). If the speedup is significant, then I'll have to bite the bullet and refactor.

Ran some simple tests today. They weren't in favor of the refactor
```
sizeof(Order): 72
Time (10 iterations): 0.497007 s (497.007 ms, 497007 µs, 4.97007e+08 ns)
Orders processed: 1000000, Trades generated: 977362, Total volume processed: 252268378

sizeof(Order): 72
Time (10 iterations): 6.72032 s (6720.32 ms, 6.72032e+06 µs, 6.72032e+09 ns)
Orders processed: 10000000, Trades generated: 9767226, Total volume processed: 2525162046

sizeof(Order): 64
Time (10 iterations): 0.503089 s (503.089 ms, 503089 µs, 5.03089e+08 ns)
Orders processed: 1000000, Trades generated: 982364, Total volume processed: 252545695

sizeof(Order): 64
Time (10 iterations): 6.80185 s (6801.85 ms, 6.80185e+06 µs, 6.80185e+09 ns)
Orders processed: 10000000, Trades generated: 9832339, Total volume processed: 2524818850
```

## Refactoring using templates
Claude made some good points today. In the matching logic, they overlap quite a bit and the only difference is the map types are different (literally just the comparator but of course they aren't compatible lol) and some minor logic difference between limit and market orders. So what I could do is use templates like so
```cpp
template<typename MapType, Order::Type OrderType>
OrderResult matchOrder(Order& order, MapType& orderMap)
```
and I don't have to rewrite the same logic for `bid_map` and `ask_map`! Also just some `if constexpr` branches for the different matching logic and my code would be hella DRY. This can also be done with any logic that requires branching simply because of the map difference like inserting into orderbook and (maybe) depth.

Made the optimization but now I'm considering the tradeoffs of using `OrderType` as a template parameter or just using runtime values from `order.type` (in `matchOrder`) so I'm noting this here so I can test in the benchmark. Claude said template would be better since it's a hot path but why not check anyways.

Another consideration is breaking up big complex functions (`placeOrder`, `matchOrderTemplate`) into smaller more understandable chunks. Function calls might incur some overhead but compilers are good at inlining so we'll have to benchmark it.

## Network vs Internal
Ah hell nah I just realized what am I even storing copies for here. If the code is used for trading sims or whatever over the network then the `OrderResult` will be serialized (and copied) before being sent anyways so I could just use references. Mayn ts pmo I'm gonna have to revert all of ts soon.

Now that I've given it more thought, for this phase, returning copies would still be good since I will be testing internally and object lifetime is an actual concern. However, when we move this to a server for actual market sims or whatever, then returning copies would be slow. When it gets to that point, we can always just template `OrderResult` to hold copies or refs based on a template parameter.

Also, most orderbook visualizations use depth a lot like every 100ms or so. If it comes to that, I might have to track depth internally after every order place and match instead of recalculating it everytime with the depth functions.
