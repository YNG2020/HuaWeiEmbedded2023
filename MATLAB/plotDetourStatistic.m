function [f_detourStatistic] = plotDetourStatistic()
    
    f_detourStatistic = figure("Name", "detourStatistic");
    set(gcf, 'WindowState', 'maximized');
    tiledlayout(3, 4, 'TileSpacing', 'tight')
    repeatTime = 100;

    for randSeed = 1 : 10
        
        [dataGenArgs] = dataGen(randSeed);
        system("..\build\x64-Release\main.exe 0 0 0 0 0" + " " + num2str(randSeed));
        % 记录每个业务在不考虑通道堵塞的条件下经过的边的编号
        fileID = fopen("output\" + num2str(randSeed) + "transactionMinPath.txt", 'r');
        transactionMinPath = cell(dataGenArgs.T, 1);
        transactionMinPathSize = zeros(dataGenArgs.T, 1);
        for i = 1 : dataGenArgs.T
            m = fscanf(fileID, '%d', 1); % 读取每一个业务的经过的最小边的数量
            edgePassed = fscanf(fileID, '%d', m); % 读取经过的边的编号
            transactionMinPath{i, 1} = edgePassed;
            transactionMinPathSize(i) = m;
        end
        fclose(fileID);
        uniqueNum = unique(transactionMinPathSize(:, 1));
        table = zeros(length(uniqueNum), 3);
        table(:, 1) = uniqueNum;
        
        for k = 1 : repeatTime
    
            system("..\build\x64-Release\main.exe 0 0 0 0 0" + " " + num2str(randSeed));
            % 打开文件
            fileID = fopen("output\" + num2str(randSeed) + "result.txt", 'r');
            
            % 记录每个业务经过的边的编号
            transactionPassEdgeID = cell(dataGenArgs.T, 1);
            transactionPassEdgeSize = zeros(dataGenArgs.T, 1);
            
            % 读取第一行，表示要加边数量Y
            newEdgesCnt = fscanf(fileID, '%d', 1);
            
            % 读取 Y 行，每行两个整数𝑠𝑖、𝑡𝑖，表示新边的起点和终点
            fscanf(fileID, '%d %d', [2, newEdgesCnt]);
            
            % 读取T个业务
            for i = 1 : dataGenArgs.T
                data = fscanf(fileID, '%d', 3); % 读取前三个整数
                m = data(2); % 经过的边数量
                n = data(3); % 经过的放大器个数
                edgePassed = fscanf(fileID, '%d', m); % 读取经过的边的编号
                transactionPassEdgeID{i, 1} = edgePassed;
                transactionPassEdgeSize(i) = m;
                fscanf(fileID, '%d', n); % 读取经过的放大器所在节点的编号
            end
            fclose(fileID);
            extraEdgeNums = transactionPassEdgeSize - transactionMinPathSize;
                    
            for i = 1 : dataGenArgs.T
                extraEdgeNum = extraEdgeNums(i);
                tableIdx = find((uniqueNum - transactionMinPathSize(i, 1)) == 0);
                table(tableIdx, 2) = table(tableIdx, 2) + 1;
                table(tableIdx, 3) = table(tableIdx, 3) + extraEdgeNum;
            end
            
        end

        nexttile
        bar(table(:, 1), table(:, 3) ./ table(:, 2));
        xlabel("Minimum used piles", 'FontSize', 12)
        ylabel("Mean extra used piles", 'FontSize', 12)
        title("Data set " + num2str(randSeed), 'FontSize', 12);
    end

end