function [edge] = Prim(args, nodeArray, startNode)

    nNode = length(nodeArray);
    nodeDis = zeros(nNode, nNode);
    for i = 1 : nNode
        for j = 1 : nNode
            nodeDis(i, j) = calDis(nodeArray{i}, nodeArray{j});
        end
    end
    
    minDist2MST = zeros(1, nNode);
    minDist2NodeInMST = zeros(1, nNode);
    for i = 1 : nNode
        minDist2MST(i) = nodeDis(startNode + 1, i); % 初始每个顶点到MST的最短路径长度为到startNode顶点的距离
        minDist2NodeInMST(i) = startNode + 1;   % 初始每个顶点到MST的最短路径的终点都是startNode
    end
    
    for i = 1 : nNode
        minDist = Inf;
        newNodeInMST = startNode + 1;
        for j = 1 : nNode
            if (minDist2MST(j) && minDist2MST(j) < minDist)
                minDist = minDist2MST(j);
                newNodeInMST = j;
            end
        end
        minDist2MST(newNodeInMST) = 0;  % 将newNode加入到MST中
    
        for j = 1 : nNode
            if minDist2MST(j) && nodeDis(newNodeInMST, j) < minDist2MST(j)
                minDist2MST(j) = nodeDis(newNodeInMST, j);
                minDist2NodeInMST(j) = newNodeInMST;
            end
        end
    
    end

    nEdge = nNode - 1;
    edge = zeros(nEdge, 3);
    edgeCnt = 1;
    rng(args.randomSeed);
    for i = 1 : nNode
        if (i == startNode + 1)
            continue;
        end
        edge(edgeCnt, 1) = i - 1;
        edge(edgeCnt, 2) = minDist2NodeInMST(i) - 1;
        edge(edgeCnt, 3) = randi(args.D);	% 保证距离大于0且不大于rD
        edgeCnt = edgeCnt + 1;
    end

end


function dist = calDis(node1, node2)
    dist = sqrt((node1.x - node2.x)^2 + (node1.y - node2.y)^2);
end