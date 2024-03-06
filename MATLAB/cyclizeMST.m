function [newEdge] = cyclizeMST(args, nodeArray, flatNodeArray, edge)
    
    rng(args.randomSeed);
    newEdge = zeros(args.nMaxNeighbor * args.nCell / 2, 3);
    edgeCnt = 0;
    % 首先获取在MST中的各个小区域间的相互跨越关系
    cell2Cell = zeros(args.nMaxNeighbor + 1, args.nCell);    % 数组的第一行保留作为计数用
    nEdge = size(edge, 1);
    for i = 1 : nEdge
        cellID1 = flatNodeArray{edge(i, 1) + 1}.cellID;
        cellID2 = flatNodeArray{edge(i, 2) + 1}.cellID;
        if cellID1 ~= cellID2
            nextPos = cell2Cell(1, cellID1);
            cell2Cell(nextPos + 2, cellID1) = cellID2;
            cell2Cell(1, cellID1) = nextPos + 1;
            nextPos = cell2Cell(1, cellID2);
            cell2Cell(nextPos + 2, cellID2) = cellID1;
            cell2Cell(1, cellID2) = nextPos + 1;
        end
    end
    
    for cellID1 = 1 : args.nCell
        if cell2Cell(1, cellID1) > 0
            [neighborID] = getNeighborCell(args, cellID1);
            nNeighbor = length(neighborID);
            for j = 1 : nNeighbor
                if rand() >= args.p_addPathCell2Cell
                    continue;
                end
                cellID2 = neighborID(j);
                passFlag = false;
                for k = 1 : cell2Cell(1, cellID1)
                    if cellID2 == cell2Cell(k + 1, cellID1) % 检查这两个区域是否本来已经有连边，有的话，跳过
                        passFlag = true;
                    end
                end
                if passFlag
                    continue;
                end
                [nodeID1, nodeID2] = getMinDistPairBetweenCell(args, cellID1, cellID2, nodeArray);
                if nodeID1 == -1
                    continue;
                end
                nextPos = cell2Cell(1, cellID1);
                cell2Cell(nextPos + 2, cellID1) = cellID2;
                cell2Cell(1, cellID1) = nextPos + 1;
                nextPos = cell2Cell(1, cellID2);
                cell2Cell(nextPos + 2, cellID2) = cellID1;
                cell2Cell(1, cellID2) = nextPos + 1;
                edgeCnt = edgeCnt + 1;
                newEdge(edgeCnt, 1) = nodeID1;
                newEdge(edgeCnt, 2) = nodeID2;
                newEdge(edgeCnt, 3) = randi(args.D);	% 保证距离大于0且不大于rD
            end
        end
    end
    newEdge = newEdge(1 : edgeCnt, :);
end

function [nodeID1, nodeID2] = getMinDistPairBetweenCell(args, cellID1, cellID2, nodeArray)
    minDist = Inf;
    nodeID1 = -1;
    nodeID2 = -1;
    for i = 1 : args.nNodeInCell
        if (nodeArray{i, cellID1}.nodeCreated)
            x1 = nodeArray{i, cellID1}.x;
            y1 = nodeArray{i, cellID1}.y;
            for j = 1 : args.nNodeInCell
                if (nodeArray{j, cellID2}.nodeCreated)
                    x2 = nodeArray{j, cellID2}.x;
                    y2 = nodeArray{j, cellID2}.y;
                    dist = sqrt((x1 - x2)^2 + (y1 - y2)^2);
                    if dist < minDist
                        minDist = dist;
                        nodeID1 = nodeArray{i, cellID1}.nodeID;
                        nodeID2 = nodeArray{j, cellID2}.nodeID;
                    end
                end
            end
        end
    end
end


function [neighborID] = getNeighborCell(args, centerCellID)
    if args.connected_8
        dirX = [-1 -1 -1 0 1 1 1 0];
        dirY = [1 0 -1 -1 -1 0 1 1];
    else
        dirX = [-1 0 1 0];
        dirY = [0 -1 0 1];
    end
    neighborID = zeros(1, length(dirX));
    nXCell = (args.spaceLimX2 - args.spaceLimX1 - args.cellX) / (args.cellX + args.cellXGap) + 1;
    nYCell = (args.spaceLimY2 - args.spaceLimY1 - args.cellY) / (args.cellY + args.cellYGap) + 1;
    xIdx = mod(centerCellID - 1, nXCell);
    yIdx = floor((centerCellID - 1) / nXCell);

    validNeighborCnt = 0;
    for i = 1 : length(dirX)
        curXIdx = xIdx + dirX(i);
        curYIdx = yIdx + dirY(i);
        if curXIdx < 0 || curXIdx > nXCell - 1 || curYIdx < 0 || curYIdx > nYCell - 1
            continue;
        end
        neighbor_id = curYIdx * nXCell + curXIdx + 1;
        validNeighborCnt = validNeighborCnt + 1;
        neighborID(validNeighborCnt) = neighbor_id;
    end
    neighborID = neighborID(1 : validNeighborCnt);
end