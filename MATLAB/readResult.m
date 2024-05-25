function [totCost, edgeStat, sortedEdgeStat, edgePile, transactionPassEdgeID, transactionMinPath] = readResult()
    % 读取原始光网络
    fileID = fopen('dataMATLAB.txt', 'r');
    
    % 读取第一行
    firstLine = fscanf(fileID, '%d', 5);
    M = firstLine(2);
    T = firstLine(3);
    P = firstLine(4);
    
    % 创建边集统计数组，第i行依次存储某条重边的起点、终点、重边上的业务数，重边数，边上通道的利用率
    edgeStat = zeros(M, 5);
    edgeStat(:, 4) = 1;

    % 记录每个业务经过的边的编号
    transactionPassEdgeID = cell(T, 1);
    
    % 读取每个业务的数据
    for j = 1 : M
        % 读取第一个整数pj
        tmpEdge = fscanf(fileID, '%d', 3);
        % 读取接下来的pj个整数
        edgeStat(j, 1) = tmpEdge(1);
        edgeStat(j, 2) = tmpEdge(2);
    end
    % 关闭文件
    fclose(fileID);
    
    % 打开文件
    fileID = fopen('output\0result.txt', 'r');
    
    % 读取第一行，表示要加边数量Y
    newEdgesCnt = fscanf(fileID, '%d', 1);
    
    % 读取 Y 行，每行两个整数𝑠𝑖、𝑡𝑖，表示新边的起点和终点
    newEdges = fscanf(fileID, '%d %d', [2, newEdgesCnt]);
    newEdges = newEdges';
    
    % 统计重边数
    for i = 1 : newEdgesCnt
        startPoint = newEdges(i, 1);
        endPoint = newEdges(i, 2);
        edgeID = find(edgeStat(:, 1) == startPoint & edgeStat(:, 2) == endPoint | ...
                    (edgeStat(:, 2) == startPoint & edgeStat(:, 1) == endPoint));
        edgeStat(edgeID, 4) = edgeStat(edgeID, 4) + 1;
    end
    
    totM = 0; totN = 0;
    
    % 读取 T 行，每行前三个整数𝑝𝑗、𝑚𝑗、𝑛𝑗，表示第 j 条业务的通道编号为𝑝𝑗、经过的边数量为𝑚𝑗、经过的放大器个数为𝑛𝑗
    newEdgeIdx = nan(M, 1);     % 用于存储新边的编号
    for i = 1 : M
        newEdgeIdx(i) = i - 1;
    end
    newEdgeTmpCnt = ones(M, 1);                   % 与上述数组配套使用
    
    edgeStat = [edgeStat zeros(M, max(edgeStat(:, 4) + 2))];     % 用于统计每一条重边上的业务量，并存储利用率最低的重边和第几次出现重边时利用率最低
    for i = 1 : T
        data = fscanf(fileID, '%d', 3); % 读取前三个整数
        p = data(1); % 通道编号
        m = data(2); % 经过的边数量
        n = data(3); % 经过的放大器个数
        totM = totM + m;
        totN = totN + n;
        edgePassed = fscanf(fileID, '%d', m); % 读取经过的边的编号
        transactionPassEdgeID{i, 1} = edgePassed;
        for j = 1 : m
            edgeID = edgePassed(j);         % 此处边的编号从0开始，需要做适应MATLAB的适配
            oriEdgeID = edgeID;             % 记录边的原本的编号
            if edgeID >= M
                edgeID = edgeID - M + 1;
                startPoint = newEdges(edgeID, 1);
                endPoint = newEdges(edgeID, 2);
            else
                edgeID = edgeID + 1;
                startPoint = edgeStat(edgeID, 1);
                endPoint = edgeStat(edgeID, 2);
            end
    
            edgeID = find((edgeStat(:, 1) == startPoint & edgeStat(:, 2) == endPoint) | ...
                    (edgeStat(:, 2) == startPoint & edgeStat(:, 1) == endPoint));
            edgeStat(edgeID, 3) = edgeStat(edgeID, 3) + 1;
    
            idx = find(newEdgeIdx(edgeID, :) == oriEdgeID);
            if isempty(idx)     % 如果新边没有添加到新边数组，把它添加进去
                newEdgeTmpCnt(edgeID) = newEdgeTmpCnt(edgeID) + 1;
                idx = newEdgeTmpCnt(edgeID);
                newEdgeIdx(edgeID, idx) = oriEdgeID;
            end
            edgeStat(edgeID, 5 + idx) = edgeStat(edgeID, 5 + idx) + 1;  % 对于重边的情况，分情况统计
        end
        amplifiers_passed = fscanf(fileID, '%d', n); % 读取经过的放大器所在节点的编号
    end

    for i = 1 : M
        for j = 2 : size(newEdgeIdx, 2)
            if newEdgeIdx(i, j) == 0
                newEdgeIdx(i, j) = nan;
            end
        end
    end
    
    % edgeStat的后续处理
    edgeStat(:, 5) = round(100 * edgeStat(:, 3) ./ (edgeStat(:, 4) * P));   % 通道利用率，用百分数表示
    [newEdgeIdx, idx] = sort(newEdgeIdx, 2);
    tmp = edgeStat(:, 6 : 6 + size(newEdgeIdx, 2) - 1);
    for i = 1 : M
        edgeStat(i, 6 : 6 + size(newEdgeIdx, 2) - 1) = tmp(i, idx(i, :));
        minTranNum = 10000000;
        edgeStat(i, 6 + size(newEdgeIdx, 2) + 1) = 1;
        for j = 1 : newEdgeTmpCnt(i)
            if minTranNum > edgeStat(i, 6 + j - 1)
                minTranNum = edgeStat(i, 6 + j - 1);
                edgeStat(i, 6 + size(newEdgeIdx, 2)) = minTranNum;
                edgeStat(i, 6 + size(newEdgeIdx, 2) + 1) = j;
            end
        end
        for j = 1 : size(newEdgeIdx, 2)
            if edgeStat(i, 6 + j - 1) == 0
                edgeStat(i, 6 + j - 1) = nan;
            end
        end
    end
    
    totCost = totM + totN * 100 + size(newEdges, 1) * 1000000;
    
    % 关闭文件
    fclose(fileID);
    sortedEdgeStat = sortrows(edgeStat, [-4 5 3 1 2]);

    % 读取每条边上的通道的使用情况，前三个数是边的起点，终点，被占用的通道数，剩下的P个数是占用某个通道的业务编号
    edgePile = zeros(M + newEdgesCnt, 3 + P);
    fileID = fopen('output\0transactionInPile.txt', 'r');
    for i = 1 : M + newEdgesCnt
        tmp = fscanf(fileID, '%d', 3 + P);
        edgePile(i, 1) = tmp(1);
        edgePile(i, 2) = tmp(2);
        edgePile(i, 3) = tmp(3);
        edgePile(i, 4 : 3 + P) = tmp(4 : 3 + P);
    end
    fclose(fileID);

    % 记录每个业务在不考虑通道堵塞的条件下经过的边的编号
    fileID = fopen('output\0transactionMinPath.txt', 'r');
    transactionMinPath = cell(T, 1);
    for i = 1 : T
        m = fscanf(fileID, '%d', 1); % 读取每一个业务的经过的最小边的数量
        edgePassed = fscanf(fileID, '%d', m); % 读取经过的边的编号
        transactionMinPath{i, 1} = edgePassed;
    end
    fclose(fileID);
    save newEdgeIdx.mat
    save newEdgeTmpCnt.mat
end

