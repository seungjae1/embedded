// bst_module.c
#include "redismodule.h"

int BSSET_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc != 4) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleString *key = argv[1];
    RedisModuleString *scoreStr = argv[2];
    RedisModuleString *value = argv[3];

    double score;
    if (RedisModule_StringToDouble(scoreStr, &score) != REDISMODULE_OK) {
        return RedisModule_ReplyWithError(ctx, "Invalid score");
    }

    // RedisModule_ZsetAdd 함수를 적절하게 사용
    RedisModuleKey *zsetKey = RedisModule_OpenKey(ctx, key, REDISMODULE_WRITE);
    int flags = REDISMODULE_ZADD_NX;  // Optional flags
    if (RedisModule_ZsetAdd(zsetKey, score, value, &flags) != REDISMODULE_OK) {
        RedisModule_CloseKey(zsetKey);
        return RedisModule_ReplyWithError(ctx, "Failed to add to the Sorted Set");
    }

    RedisModule_ReplyWithSimpleString(ctx, "OK");
    RedisModule_CloseKey(zsetKey);
    return REDISMODULE_OK;
}

int BSGET_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc != 2) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleString *key = argv[1];

    RedisModuleCallReply *reply = RedisModule_Call(ctx, "ZRANGEBYSCORE", "crr", key, "-inf", "+inf");
    if (reply == NULL || RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_ERROR) {
        return RedisModule_ReplyWithError(ctx, "Failed to retrieve from the Sorted Set");
    }

    RedisModule_ReplyWithCallReply(ctx, reply);
    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (RedisModule_Init(ctx, "bst_module", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "BSSET", BSSET_RedisCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "BSGET", BSGET_RedisCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

