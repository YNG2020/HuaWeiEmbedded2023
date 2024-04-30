function trans = generateTransaction(args, flatNodeArray, nodeArray)
    cellDist = zeros(args.nCell, args.nCell);
    nXCell = (args.spaceLimX2 - args.spaceLimX1 - args.cellX) / (args.cellX + args.cellXGap) + 1;
    trans = zeros(args.TMAX, 2);
    
    for i = 1 : args.nCell
        cellID1 = i;
        xIDx1 = mod(cellID1 - 1, nXCell);
        yIDx1 = floor((cellID1 - 1) / nXCell);
        for j = 1 : args.nCell
            cellID2 = j;
            xIDx2 = mod(cellID2 - 1, nXCell);
            yIDx2 = floor((cellID2 - 1) / nXCell);
            if args.connected_8 % 如果在环化时，是往8连通方向进行搜索，则使用最大距离衡量cell之间的距离
                dist = max(abs(xIDx2 - xIDx1), abs(yIDx2 - yIDx1));
            else    % 反之，使用曼哈顿距离衡量cell之间的距离
                dist = abs(xIDx2 - xIDx1) + abs(yIDx2 - yIDx1);
            end
            cellDist(i, j) = dist;
        end
    end
    [~, cellDistSortedIDx] = sort(cellDist, 2, 'ascend');
    rng(args.randomSeed);
    transactionCnt = 0;
    rngSeedCnt = randi(86400);
    for i = 1 : length(flatNodeArray)
        nodeID1 = flatNodeArray{1, i}.nodeID;
        cellID1 = flatNodeArray{1, i}.cellID;
        [nodeID2, rngSeedCnt] = getNodeIDAccordingDistMode(args, cellDistSortedIDx, nodeArray, cellID1, rngSeedCnt, nodeID1);
        rng(args.randomSeed + rngSeedCnt);
        nRepeated = ceil(randi(args.P) * args.transactionPressure * 0.5);
        for j = 1 : nRepeated
            transactionCnt = transactionCnt + 1;
            trans(transactionCnt, 1) = nodeID1;
            trans(transactionCnt, 2) = nodeID2;
        end
    end
    trans = trans(1:transactionCnt, :);
    args.T = transactionCnt;
end

function [nodeID2, rngSeedCnt] = getNodeIDAccordingDistMode(args, cellDistSortedIDx, nodeArray, cellID1, rngSeedCnt, nodeID1)
    accumulate_p_distMode = cumsum(args.p_distMode);    % 将概率逐个累加，便于轮盘选
    nodeID2 = -1;
    n_interval = length(args.p_distMode) - 1;
    while nodeID2 == -1     % 循环，直到找到合适的nodeID
        rngSeedCnt = rngSeedCnt + 1;  % 保证每次循环生成的随机数都不一样
        rng(args.randomSeed + rngSeedCnt);
        randValue = rand();
        for i = 1 : args.num_distMode
            if randValue <= accumulate_p_distMode(i)
                mode = i;
                break;
            end
        end
        if mode == 1
            cellID2 = cellDistSortedIDx(cellID1, 1);
            nodeIDxArray = randperm(args.nNodeInCell); % 保证节点选取的随机性
            for i = 1 : args.nNodeInCell
                if nodeArray{nodeIDxArray(i), cellID2}.nodeCreated
                    if nodeArray{nodeIDxArray(i), cellID2}.nodeID == nodeID1
                        continue;
                    end
                    nodeID2 = nodeArray{nodeIDxArray(i), cellID2}.nodeID;
                    break;
                end
            end
        else
            if mode <= n_interval
                n_interval_long = floor((args.nCell - 1) / n_interval);
            else
                n_interval_long = args.nCell - floor((args.nCell - 1) / n_interval) * n_interval - 1;
            end
            randIDX = 1 + (mode - 2) * n_interval_long + randi(n_interval_long); 
            cellID2 = cellDistSortedIDx(cellID1, randIDX);    % 保证区间选取的随机性
            nodeIDxArray = randperm(args.nNodeInCell); % 保证节点选取的随机性
            for i = 1 : args.nNodeInCell
                if nodeArray{nodeIDxArray(i), cellID2}.nodeCreated
                    nodeID2 = nodeArray{nodeIDxArray(i), cellID2}.nodeID;
                    break;
                end
            end
        end
    end
end