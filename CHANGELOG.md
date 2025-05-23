# This is an unofficial changelog I keep to document the trials and tribulations of this project

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

## Thoughts

### Cache alignment
Started fussing over cache alignment here. `Order` objects are already 64 bytes (perfect!) but `Trade`s are 48 bytes but their corresponding id's are 16 bytes. Thing is, there's no reason to store the trade id's in `idPool` since no other objects point to them except the `Trade`s themselves which persists in the `tradeList`. There are two approaches I considered here
- Store `uuid` directly in `Trade` instead of a pointer. Pros: Pads up to 56 bytes, doesn't require dereferencing to get `id`. Cons: Have to change current implementation and tests
- When I get to the memory allocator part, make sure each `Trade` objects are stored next to their id in memory. Pros: Pads up exactly to 64 bytes, no need to change implementation except maybe change tests that check trade id is stored in `idPool`. Cons: Way more complicated and probably wouldn't be worth it

As I thought about the second approach more, I started to think would optimizing the `Trade` placement in memory for cache alignment be worth it? After all, `Trade`s aren't a big part of the `OrderBook` operations anyways. They are created once when `Order`s match, stored in `tradeList` for bookkeeping and sent as copies to users. After that, no further references to them are made except probably when they are flushed into a database. I'll take a look again once I've got the basic implementation done and get some benchmarks setup but this thought process is good to document so I don't have to think about it all over again in case `Trade` memory alignment do become a bottleneck for some reason.

Man now the `Order` is no longer 64 bytes. Some ways to make it work is maybe using an `int32_t` to store the price and just divide by a precision value. But this has some problems:
- If we just use a universal precision like 0.0001 (penny stocks precision) then we can just multiply the price by 0.0001 everytime we need to use it and we can live happily ever after. Thing is, BRK.A trades for ~$700k which would be ~7,000,000,000 with this system, and if you haven't realized, a 32-bit int can only store up to ~2,000,000,000 and even its unsigned counterpart can only store double that. If we use a 64-bit int then that would be just as big as a double.
- If we use a relative precision then it would solve this previous problem. If an order is 90c then its precision would be 0.0001 so it would be 900,000. If an order is $9000 then its precision would be 0.01 so it would be 900,000 as well. The question is how does an `OrderBook` tell these apart? If the `tickSize` is 0.01 or 0.0001 then sure, but what if its 0.001? 0.05? 1? These wouldn't work unless I have another member which tracks the precision which then would make the `Order` just as big as it was.
