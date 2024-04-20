% 打开文件
fileID = fopen('result.txt', 'r');

% 读取第一行，表示要加边数量Y
newEdgesCnt = fscanf(fileID, '%d', 1);

% 读取 Y 行，每行两个整数𝑠𝑖、𝑡𝑖，表示新边的起点和终点
newEdges = fscanf(fileID, '%d %d', [2, newEdgesCnt]);
newEdges = newEdges';

totM = 0; totN = 0;

% 读取 T 行，每行前三个整数𝑝𝑗、𝑚𝑗、𝑛𝑗，表示第 j 条业务的通道编号为𝑝𝑗、经过的边数量为𝑚𝑗、经过的放大器个数为𝑛𝑗
for i = 1:dataGenArgs.T
    data = fscanf(fileID, '%d', 3); % 读取前三个整数
    p = data(1); % 通道编号
    m = data(2); % 经过的边数量
    n = data(3); % 经过的放大器个数
    totM = totM + m;
    totN = totN + n;
    edges_passed = fscanf(fileID, '%d', m); % 读取经过的边的编号
    amplifiers_passed = fscanf(fileID, '%d', n); % 读取经过的放大器所在节点的编号
end

totCost = totM + totN * 100 + size(newEdges, 1) * 1000000;

% 关闭文件
fclose(fileID);
