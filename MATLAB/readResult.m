% 读取原始光网络
fileID = fopen('dataMATLAB.txt', 'r');

% 读取第一行
firstLine = fscanf(fileID, '%d', 5);
N = firstLine(1);
M = firstLine(2);
T = firstLine(3);
P = firstLine(4);

% 创建边集统计数组，第i行依次存储某条重边的起点、终点、重边上的业务数，重边数，边上通道的利用率
edgeStat = zeros(M, 5);
edgeStat(:, 4) = 1;

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
fileID = fopen('result.txt', 'r');

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
    % edgeID = find(edgeStat(:, 1) == startPoint & edgeStat(:, 2) == endPoint);
    if isempty(edgeID)
        a = 1;
    end
    edgeStat(edgeID, 4) = edgeStat(edgeID, 4) + 1;
end

totM = 0; totN = 0;

% 读取 T 行，每行前三个整数𝑝𝑗、𝑚𝑗、𝑛𝑗，表示第 j 条业务的通道编号为𝑝𝑗、经过的边数量为𝑚𝑗、经过的放大器个数为𝑛𝑗
for i = 1 : T
    data = fscanf(fileID, '%d', 3); % 读取前三个整数
    p = data(1); % 通道编号
    m = data(2); % 经过的边数量
    n = data(3); % 经过的放大器个数
    totM = totM + m;
    totN = totN + n;
    edgePassed = fscanf(fileID, '%d', m); % 读取经过的边的编号
    for j = 1 : m
        edgeID = edgePassed(j);         % 此处边的编号从0开始，需要做适应MATLAB的适配
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
    end
    amplifiers_passed = fscanf(fileID, '%d', n); % 读取经过的放大器所在节点的编号
end
edgeStat(:, 5) = round(100 * edgeStat(:, 3) ./ (edgeStat(:, 4) * P));   % 通道利用率，用百分数表示
totEdgeID = unique(totEdgeID);

totCost = totM + totN * 100 + size(newEdges, 1) * 1000000;

% 关闭文件
fclose(fileID);

load flatNodeArray.mat
BusinessAllocation = figure("Name", "Edge Statistic");
ax = axes('Parent', BusinessAllocation);
for k = 1 : 3
    maxVal = max(edgeStat(:, 2 + k));
    cmap = turbo(maxVal + 1); % 定义颜色映射（可以根据实际需求选择其他颜色映射）
    
    hold on
    % 绘制每条边
    for i = 1 : size(edgeStat, 1)
        % 根据权值确定颜色深浅
        colorIndex = edgeStat(i, 2 + k) + 1;
        color = cmap(colorIndex, :);
        lineWidth = 1 + 3 * ((colorIndex + 1) / (maxVal + 1));
        plot3([flatNodeArray{edgeStat(i, 1) + 1}.x, flatNodeArray{edgeStat(i, 2) + 1}.x], ...
        [flatNodeArray{edgeStat(i, 1) + 1}.y, flatNodeArray{edgeStat(i, 2) + 1}.y], ...
        k * ones(1, 2), '-o', 'Color', color, 'LineWidth', lineWidth, ...
         'MarkerSize', 3,...
        'MarkerEdgeColor', 'b',...
        'MarkerFaceColor', 'b')
    end
    colormap(cmap);
    colorbarHandle = colorbar;
    colorbarHandle.Label.String = 'Number of transactions';
    colorbarHandle.FontSize = 11;
    colorbarHandle.Label.FontSize = 14;
    clim([0, maxVal]);
    axis off
    % 启用数据提示模式
    dcm = datacursormode(gcf);
    set(dcm, 'UpdateFcn', {@customGraphDatatip, nodeArray, args});
    set(gcf, 'Color', 'white');
end

sortedEdgeStat = sortrows(edgeStat, [-4 5 3 1 2]);